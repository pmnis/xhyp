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

/*
 * SYSTEM CALLS
 */

/*
 * hyp_idle:
 * 	Set current partition to stop
 */
int hyp_idle(void)
{
	debsched("SP at.....: %08lx\n", _context->sregs.sp);
	debsched("LR at.....: %08lx\n", _context->sregs.lr);
	debsched("PC at.....: %08lx\n", _context->sregs.pc);
	sched->sleep(current);
	return 0;
}

/*
 * hyp_preempt_disable
 * 	Only available for drivers
 * 	delays the preemption until next preempt_enable
 */

int hyp_preempt_disable(void)
{
	sched->preempt_count++;
	return sched->preempt_count;
}

/*
 * hyp_preempt_enable
 * 	Only available for drivers
 * 	end of critical section, allow preemption
 * 	if need_resched has been set call schedule()
 */

int hyp_preempt_enable(void)
{
	sched->preempt_count--;
	if (sched->need_resched && (!sched->preempt_count))
		schedule();
	return 0;
}


/*
 * function: schedule:
 * purpose :
 * 	put back current on his queue
 * 	take domain with higher priority
 */

void schedule(void)
{
	debsched("\n");
	if (current->id) event_new(EVT_SCHEDOUT);

	/* Choose the new current domain 		*/
	/* Wait for interrupt if no domain found	*/
	while (!sched->get())
		wfi();

	debsched("current: %d\n", current->id);

	/* now we got the new current we wont schedule for a while */
	sched->need_resched = 0;

	/* Wakeup current domain	*/
	event_new(EVT_SCHEDIN);
	debsched("wakeup at %08lx\n", current->ctx.sregs.pc);
	sched->wake(current);
}

