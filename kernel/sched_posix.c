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

static struct runqueue runqueue;

/*
 * Initialization function for the scheduler
 */

static void sched_init(void)
{
	int i;

#ifdef CONFIG_ARINC
	scheduler.type = SCHED_ARINC;
#endif
	/* Initialize runqueue          */
	for (i = 0; i < 32; i++)
		list_init(&runqueue.list[i]);
	runqueue.bitmap = 0;
}

static void sched_add(struct domain *d)
{
	debsched("adding %d\n", d->id);
	list_add_tail(&d->list, &runqueue.list[d->prio]);
	runqueue.bitmap |= 0x01 << d->prio ;
	debsched("runqueue.bitmap: %08lx\n", runqueue.bitmap);
}

static void sched_delete(struct domain *d)
{
	debsched("delete %d\n", d->id);
	list_del(&d->list);
	list_init(&d->list);
	if (list_empty(&runqueue.list[d->prio])) {
		runqueue.bitmap &= ~(0x01 << d->prio );
	}
}

static inline void print_state (struct domain *d)
{
	if (!(debug_level & DEB_SCHED)) return;

	printk("[%d] : ", d->id);
	if (d->state == DSTATE_RUN) printk("R");
	else printk(".");
	if (d->state == DSTATE_SLEEP) printk("S");
	else printk(".");
	if (d->state == DSTATE_READY) printk("Y");
	else printk(".");
	if (d->state == DSTATE_DEAD) printk("D");
	else printk(".");
	if (d->state == DSTATE_STOP) printk("P");
	else printk(".");
	printk(" [%08lx][%08lx]", d->sp->v_irq_pending, d->sp->v_irq_enabled);
	printk("\n");
}

static void sched_put(struct domain *d)
{
	debsched("put %d\n", d->id);
	list_del(&d->list);
	list_init(&d->list);
	list_add_tail(&d->list, &runqueue.list[d->prio]);
}

static int sched_get(void)
{
	int i;

	current = idle_domain;
	for (i=0; i< 32; i++) {
		if (runqueue.bitmap & (0x01 << i)) {
			if (list_empty(&runqueue.list[i]))
				continue;
			current = (struct domain *)(runqueue.list[i].next);
			if (!(current->state && DSTATE_RMASK)) {
				debpanic("Bad state in scheduler: %08lx\n", current->state);
				current = idle_domain;
				continue;
			}
			debsched("bitmap: %08lx got %d rest %d\n", runqueue.bitmap, current->id, current->allocated_slices);
			break;
		}
	}
	debsched("id %d state: %d\n", current->id, current->state);
	return current->id;
}

static void sched_sleep(struct domain *d)
{
	d->state = DSTATE_SLEEP;
	sched_delete(d);
	d->ctx = *_context;
	//if (d->id > 0 ) debinfo("\n");
	//if (d->id > 0 ) show_ctx(&d->ctx);
	schedule();
}

static void sched_kill(struct domain *d)
{
	debinfo("killing %d\n", d->id);
	d->state = DSTATE_DEAD;
	sched_delete(d);
	d->ctx = *_context;
	schedule();
}

static void sched_stop(struct domain *d)
{
	d->state = DSTATE_STOP;
	sched_delete(d);
	d->ctx = *_context;
	schedule();
}

static void sched_yield(void)
{
	sched_delete(current);
	current->ctx = *_context;
	sched_add(current);
	current->state = DSTATE_READY;
	schedule();
}

static void sched_wakeup(struct domain *d)
{
/*
	if (d->id == 1) {
		debsched("d[1]: state %08lx mode %08lx\n", d->state, d->mode);
		if (d->mode == DMODE_IRQ) {
			debpanic("STOP\n");
			while (1);
		}
	}
*/
	d->state = DSTATE_RUN;
	*_context = d->ctx;
	debsched("PC %08lx SP %08lx LR %08lx PSR %08lx\n",
		_context->sregs.pc, _context->sregs.sp, _context->sregs.lr, _context->sregs.spsr);
	switch_to();
}

static void sched_dom(struct domain *d)
{
	if (d->state != DSTATE_READY)
		return;
	d->allocated_slices = 1; //d->budget;
	debsched("------allocated slices: %d\n", current->allocated_slices);
	sched_add(d);
}
/*
 * Simple time slice handling
 */
static void sched_slice(void)
{
	if (!current->id) return;
	current->slices++;
	current->allocated_slices -= 1;
	debsched("------allocated slices: %d\n", current->allocated_slices);
	if (current->allocated_slices > 0)
		return;
	current->allocated_slices = current->budget;
	debsched("------New budget      : %d\n", current->budget);
	sched_put(current);
	sched->need_resched = 1;
}

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
};

struct xhyp_scheduler *sched = &sched_posix;

