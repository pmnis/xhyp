/*
 * sched_arinc.c
 *
 * ARINC Scheduler
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

#include <xhyp/arinc.h>


void arinc_error(int i, char *reason)
{
	if (i >= 0) debsched("SLOT[%02d]: %s\n", i, reason);
	else debsched("%s\n", reason);
	debpanic("STOP");
	while(1);
}
/*
 * Initialization function for the scheduler
 */

static void sched_init(void)
{
	int i;
	int t0, t1;

	/* verify frame          */
	for (i = 0, t0 = 0; i < major->minor_count; i++) {
		t1 = major->minor[i].slot_start;
		debsched("%d: slot start: %d\n", i, t1);
		if (t1 < t0) arinc_error(i, "BAD SLOT AT INIT");
		t0 = t1 + major->minor[i].slot_size;
		debsched("%d: total size: %d\n", i, t0);
	}
	debsched("total size: %d\n", t0);
	if (t0 > major->frame_size) arinc_error(-1, "MAJOR FRAME OVERFLOW");
	debsched("%s ... ready\n", sched->name);
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

static void sched_add(struct domain *d)
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

static void sched_put(struct domain *d)
{
	debsched("\n");
}

static int sched_get(void)
{
	int i;
	int slice;
	struct domain *d;

	current = idle_domain;
	slice = jiffies % major->frame_period;
	debsched("slice %d\n", slice);

	/* Ignore slices before start and after end	*/
	if (slice < major->frame_start)
		return 0;
	if (slice > (major->frame_start + major->frame_size))
		return 0;
	/* Reset slice number to start of frame		*/
	slice -= major->frame_start;
	for (i = 0; i < major->minor_count; i++) {
		debsched("minor %d\n", i);
		if (slice < major->minor[i].slot_start) {
			debsched("minor %d slice %d slot_start %d\n", i, slice, major->minor[i].slot_start);
			return 0;
		}
		debsched("minor %d\n", i);
		if (slice < ((major->minor[i].slot_start + major->minor[i].slot_size))) {
			d = &domain_table[major->minor[i].dom_id];
			debsched("minor %d slice %d slot_start %d state %08lx\n", i, slice, major->minor[i].slot_start, d->state);
			if (d->state == DSTATE_READY) {
				debsched("........... OK: %d\n", d->id);
				current = d;
				return 1;
			} else {
				debsched("...........BAD: %d STATE %d\n", d->id, d->state);
			}
		}
	}
	debsched("NO FRAME FOUND\n");
	return 0;

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
		debsched("E %08lx P %08lx M %08lx\n", s->v_irq_enabled, s->v_irq_pending, s->v_irq_mask);
		if (s->v_irq_enabled & s->v_irq_pending & ~s->v_irq_mask)
			mode_new(d, DMODE_IRQ);
	default:
		break;
	}
	d->state = DSTATE_RUN;
}

static void sched_dom(struct domain *d)
{
	d->state = DSTATE_READY;
	debsched("domain %d ready\n", d->id);
}
/*
 * Simple time slice handling
 */
static void sched_slice(void)
{
	debsched("\n");
	sched->need_resched = 1;
	current->state = DSTATE_READY;
	current->slices++;
}

static struct xhyp_scheduler sched_arinc = {
	.need_resched = 0,
	.preempt_count = 0,
	.type = SCHED_ARINC,
	.name = "ARINC Scheduler",
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

struct xhyp_scheduler *sched = &sched_arinc;

