/*
 * sched_edf.c
 *
 * EDF/CBS Scheduler
 *
 * Author: Pierre Morel <pmorel@mnis.fr>
 *
 * $LICENSE:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <xhyp/sched.h>
#include <xhyp/timer.h>
#include <xhyp/event.h>
#include <xhyp/hyp.h>
#include <xhyp/irq.h>
#include <xhyp/errno.h>

void edf_error(int i, char *reason)
{
	debsched("SLOT[%02d]: %s\n", i, reason);
	debpanic("STOP");
	while(1);
}

static void sched_add_to_sleepq(struct domain *d)
{
	debsched("\n");
	d->state = DSTATE_SLEEP;
}

static void sched_add_from_sleepq(struct domain *d)
{
	debsched("\n");
	d->state = DSTATE_READY;
}

static void sched_delete(struct domain *d)
{
	debsched("\n");
	d->state = DSTATE_DEAD;
	if (d == current)
		context_save();
}

static int min_period;
static int max_budget;
static int sync_period;
#define MAX_PERIODS 13
#define MAX_BUDGETS 8
static int periods[MAX_PERIODS] = { 5, 10, 20, 50, 100, 200, 300, 400, 500, 600, 700, 800, 1000 };
static int budgets[MAX_BUDGETS] = { 1, 2, 5, 10, 20, 50, 100, 200 };

static int bad_budget(int b)
{
	int i;

	for (i = 0; i < MAX_BUDGETS; i++)
		if (b == budgets[i])
			return 0;
	return -EINVAL;
}
/*
 * new_sync_period:
 * - Verify period validity
 */
static int new_sync_period(int p)
{
	int i;
	int a,b;

	for (i = 0; i < MAX_PERIODS && periods[i] != p; i++)
		;
	if (i >= MAX_PERIODS)
		return -EINVAL;

	if (!sync_period || p == sync_period) {
		sync_period = p;
		return 0;
	}
	a = sync_period;
	b = p;
	do {
		if ( a < b )
			a += sync_period;
		if ( b < a )
			b += p;
		if (a == b)
			break;
	} while (1);
	sync_period = a;
	return 0;
}

static struct list task_list;

static struct domain *edf_init_deadlines(void)
{
	struct domain *d;
	struct list *l;

	list_for_each(l, &task_list) {
		d = list_entry(l, struct domain, list);
		d->deadline = d->original_deadline;
		d->slice_start = 0;
	}
	return d;
}

static struct domain *edf_get_earliest(void)
{
	struct domain *d, *candidate;
	struct list *l;

	list_for_each(l, &task_list) {
		d = list_entry(l, struct domain, list);

		if ((!candidate) || (d->deadline < candidate->deadline))
			candidate = d;
	}
	return candidate;
}

static int edf_schedulable(void)
{
	struct domain *d;
	int t = 0;

	edf_init_deadlines();
	while (t < sync_period) {
		d = edf_get_earliest();
		if ((t + d->budget) > d->deadline)
			return -E2BIG;
		if (t < d->slice_start)
			t = d->slice_start;
		t += d->budget;
		d->slice_start += d->period;
		d->deadline += d->period;
	}
	edf_init_deadlines();
	return 0;
}

/*
 * add_to_sched: Verify the schedulability of the domain
 * We Accept only a maximal period of 1s in a set of
 * 5, 10, 20, 50, 100, and multiples of 100 ms
 * and budgets of 1, 2, 5, 10, 20, 50, 80, 100, 200, ms
 * which makes 1000 slices to schedule.
 */
static int add_to_sched(struct domain *d)
{
	if (min_period > d->period)
		min_period = d->period;

	if (max_budget < d->budget)
		max_budget = d->budget;

	if (max_budget > min_period)
		goto error;

	if (new_sync_period(d->period))
		goto error;

	if (bad_budget(d->budget))
		goto error;

	list_add_tail(&d->list, &task_list);
	if (edf_schedulable() == 0)
		return 0;
	list_del(&d->list);

error:
	return -EINVAL;
}

static int sched_get(void)
{
	struct domain *d = edf_get_earliest();

	if (!d)
		return 0;
	if (d->slice_start > jiffies)
		return 0;
	return d->id;
}

static void sched_put(struct domain *d)
{
	debsched("\n");
	d->deadline += d->period;
	d->slice_start += d->period;
}

static void sched_sleep(struct domain *d)
{
	debsched("\n");
	d->state = DSTATE_SLEEP;
	schedule();
}

static void sched_kill(struct domain *d)
{
	debsched("\n");
	d->state = DSTATE_DEAD;
	schedule();
}

static void sched_stop(struct domain *d)
{
	debsched("\n");
	d->state = DSTATE_STOP;
	schedule();
}

static void sched_yield(void)
{
	debsched("\n");
	current->state = DSTATE_READY;
	schedule();
}

static void sched_wakeup(struct domain *d)
{
	struct shared_page *s;

	debsched("\n");
	switch (d->mode) {
	case DMODE_SVC:
	case DMODE_USR:
		s = d->sp;
		debsched("E %08lx P %08lx M %08lx\n", s->v_irq_enabled,
			 s->v_irq_pending, s->v_irq_mask);
		if (s->v_irq_enabled & s->v_irq_pending & ~s->v_irq_mask)
			mode_new(d, DMODE_IRQ);
	default:
		break;
	}
	d->state = DSTATE_RUN;
}

/*
 * sched_add: Called on hypercall
 * Currently not implemented
 */
static int sched_add(struct domain *d)
{
	debsched("adding domain %d : only static EDF\n", d->id);
	return -EFAULT;
}

/*
 * sched_dom: Called on initialization
 * Verify the schedulability.
 */
static int sched_dom(struct domain *d)
{
	if (add_to_sched(d) < 0) {
		debsched("adding domain %d : overschdule\n", d->id);
		return -EFAULT;
	}
	d->state = DSTATE_READY;
	debsched("domain %d ready\n", d->id);
	return 0;
}
/*
 * sched_slice: called on timer tics
 * Nothing to do but ask for rescheduling.
 */
static void sched_slice(void)
{
	debsched("\n");
	sched->need_resched = 1;
}

/*
 * Initialization function for the scheduler
 */
static void sched_init(void)
{
	debsched("%s ... ready\n", sched->name);
	list_init(&task_list);
}

static struct xhyp_scheduler sched_edf = {
	.need_resched = 0,
	.preempt_count = 0,
	.type = SCHED_EDF,
	.name = "EDF Scheduler",
	.add = sched_add,
	.delete = sched_delete,
	.put = sched_put,
	.get = sched_get,
	.sleep = sched_sleep,
	.wake = sched_wakeup,
	.stop = sched_stop,
	.yield = sched_yield,
	.start = sched_dom,
	.kill = sched_kill,
	.slice = sched_slice,
	.init = sched_init,
	.add_to_sleep = sched_add_to_sleepq,
	.add_from_sleep = sched_add_from_sleepq,
};

struct xhyp_scheduler *sched = &sched_edf;

