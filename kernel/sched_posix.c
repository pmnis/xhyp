/*
 * Scheduler.c
 *
 * Scheduling functions
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

static struct runqueue runqueue;
struct list sleep_queue;

/*
 * Initialization function for the scheduler
 */

static void sched_init(void)
{
	int i;

	/* Initialize runqueue          */
	for (i = 0; i < 32; i++)
		list_init(&runqueue.list[i]);
	runqueue.bitmap = 0;
	list_init(&sleep_queue);
}

/*
 * These are called from interrupt context
 * static void sched_add_to_sleepq(struct domain *d)
 * static void sched_add_from_sleepq(struct domain *d)
 */

static void sched_add_to_sleepq(struct domain *d)
{
	d->state = DSTATE_SLEEP;
	list_add_tail(&d->list, &sleep_queue);
}

static void sched_add_from_sleepq(struct domain *d)
{
	d->state = DSTATE_READY;
	list_del(&d->list);
	list_init(&d->list);
	list_add_tail(&d->list, &runqueue.list[d->prio]);
	runqueue.bitmap |= 0x01 << d->prio ;
}

static int sched_add(struct domain *d)
{
	debsched("adding %d\n", d->id);

	d->state = DSTATE_READY;
	list_add_tail(&d->list, &runqueue.list[d->prio]);
	runqueue.bitmap |= 0x01 << d->prio ;

	debsched("runqueue.bitmap: %08lx\n", runqueue.bitmap);
	return 0;
}

static void sched_delete(struct domain *d)
{
	debsched("delete %d\n", d->id);

	d->state = DSTATE_DEAD;
	list_del(&d->list);
	list_init(&d->list);

	if (list_empty(&runqueue.list[d->prio]))
		runqueue.bitmap &= ~(0x01 << d->prio );

	if (d == current)
		context_save();
}

static inline void print_state (struct domain *d)
{
	if (!(debug_level & DEB_SCHED))
		return;

	switch (d->state) {
	case DSTATE_RUN:
		printk("[%d] : R", d->id);
		break;
	case DSTATE_SLEEP:
		printk("[%d] : S", d->id);
		break;
	case DSTATE_READY:
		printk("[%d] : Y", d->id);
		break;
	case DSTATE_DEAD:
		printk("[%d] : D", d->id);
		break;
	case DSTATE_STOP:
		printk("[%d] : P", d->id);
		break;
	}
	printk(" [%08lx][%08lx]", d->sp->v_irq_pending, d->sp->v_irq_enabled);
	printk("\n");
}

static void sched_show_d(void)
{
	struct domain *d1, *d2;

	d1 = (struct domain *)(runqueue.list[8].next);
	d2 = (struct domain *)(runqueue.list[8].next->next);

	debsched("first %d next %d\n", d1->id, d2->id);
}

#ifdef CONFIG_SCHED_POLICY_RR
static void sched_put(struct domain *d)
{
	if (!d->id)	/* do not touch idle	*/
		return;
	debsched("d->allocated_slices: %d\n", d->allocated_slices);
	if (d->allocated_slices > 0 )
		return;
	d->allocated_slices = d->budget;
	sched_delete(d);
	sched_add(d);
}
#else
static void sched_put(struct domain *d)
{
	debsched("d->id %d\n", d->id);
	sched_show_d();
}
#endif

static int sched_get(void)
{
	int i;
	struct domain *d;

	current = idle_domain;
	for (i=0; i< 32; i++) {
		if (runqueue.bitmap & (0x01 << i)) {
			if (list_empty(&runqueue.list[i]))
				continue;
			current = (struct domain *)(runqueue.list[i].next);
			d = (struct domain *)(runqueue.list[i].next->next);
			debsched("first %d next %d\n", current->id, d->id);
			if (!(current->state && DSTATE_RMASK)) {
				debpanic("Bad state in scheduler: %08lx\n", current->state);
				current = idle_domain;
				continue;
			}
			break;
		}
	}
	return current->id;
}

static void sched_sleep(struct domain *d)
{
	if (d->id > 0 )
		debsched("\n");
	sched_delete(d);
	sched_add_to_sleepq(d);
	schedule();
}

static void sched_kill(struct domain *d)
{
	if (d->id > 0 )
		debsched("\n");
	sched_delete(d);
	schedule();
}

static void sched_stop(struct domain *d)
{
	if (d->id > 0 )
		debsched("\n");
	sched_delete(d);
	d->state = DSTATE_STOP;
	schedule();
}

static void sched_yield(void)
{
	debsched("\n");
	sched_delete(current);
	sched_add(current);
	current->state = DSTATE_READY;
	schedule();
}

static void sched_wakeup(struct domain *d)
{
	struct shared_page *s;

	debsched("id: %d\n", d->id);
	sched_show_d();
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
	debsched("id: %d\n", d->id);
	sched_show_d();
}

static int sched_dom(struct domain *d)
{
	if (d->state != DSTATE_READY)
		return -EINVAL;
	d->allocated_slices = 1;
	d->budget = 1;
	debsched("------allocated slices: %d\n", d->allocated_slices);
	sched_add(d);
	return 0;
}
/*
 * Simple time slice handling
 */
#ifdef CONFIG_SCHED_POLICY_RR
static void sched_slice(void)
{
	if (!current->id)
		return;
	current->slices++;
	current->allocated_slices--;
	if (current->allocated_slices > 0)
		return;
	sched->need_resched++;
}
#else
static void sched_slice(void)
{
	current->slices++;
	debirq("slice: %d\n", current->slices);
	return;
}
#endif

static struct xhyp_scheduler sched_posix = {
	.need_resched = 0,
	.preempt_count = 0,
	.type = SCHED_POSIX,
	.name = "POSIX Scheduler",
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

struct xhyp_scheduler *sched = &sched_posix;

