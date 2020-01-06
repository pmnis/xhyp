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
 * Context manipulation
 */

unsigned long context_sum(struct context *ctx)
{
	int i;
	unsigned long sum;
	unsigned long *p;

	p = (unsigned long *)ctx;
	for (i = 0, sum = 0 ; i < 17; i++, p++)
		sum += *p;

	return sum;
}

void context_trace(struct context *ctx)
{
	debctx("Context sum: %08lx ------- R0 %08lx\n", context_sum(ctx), ctx->regs.regs[0]);
}

/** @fn int context_verify()
 * @brief insure that the context is in non priviledge mode
 */
int context_verify(void)
{
	/* force SPSR to user mode      */
	_context->sregs.spsr &= ~mask_domain;
	_context->sregs.spsr |= mode_domain;
	return 0;
}

/** @fn int context_save()
 * @brief save context in domain
 */
void context_save(void)
{
	debctx("flags %x current->ctx_level: %d\n", current->flags, current->ctx_level);
	if (current->ctx_level++) {
		debpanic("current->ctx_level: %d\n", current->ctx_level);
		event_dump_last(20);
		while(1);
	}
	current->ctx = *_context;

	current->d_sum = context_sum(_context);

	//debctx("______________________________saved sum %08lx\n", current->d_sum);
	//debctx("SPSR.................... %08lx\n", _context->sregs.spsr);
}

/** @fn int context_restore()
 * @brief save context in domain
 */
void context_restore(void)
{
	unsigned long sum;

	debctx("flags %x current->ctx_level: %d\n", current->flags, current->ctx_level);
	if (--current->ctx_level) {
		debpanic("current->ctx_level: %d\n", current->ctx_level);
		event_dump_last(20);
		while(1);
	}

	*_context = current->ctx;

	if (current->no_check) {
		current->no_check = 0;
		return;
	}

	sum = context_sum(_context);

	if (current->d_sum != sum) {
		unsigned long *p;
		int i;
		debpanic("BAD sum: %08lx saved sum %08lx\n", sum, current->d_sum);
		debpanic("SPSR.................... %08lx\n", _context->sregs.spsr);
		for (i=0, p = (unsigned long *)_context; i < 17; i++, p++)
			debpanic("R[%02d]: %08lx\n", i, *p);
		event_dump_last(20);
		while(1);
	}
	//debctx("______________________________ sum %08lx\n", sum);
}

/*
 * SYSTEM CALLS
 */

/*
 * hyp_idle:
 * 	Set current partition to stop
 */
int hyp_idle(void)
{
	//debsched("SP at.....: %08lx\n", _context->sregs.sp);
	//debsched("LR at.....: %08lx\n", _context->sregs.lr);
	//debsched("PC at.....: %08lx\n", _context->sregs.pc);
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
	static int task_running;

	debsched("schedout\n");
	event_new(EVT_SCHEDOUT);

	/* throw away last domain to get a new one	*/
	if (task_running && sched->put)
		sched->put(current);

	/* Be sure to have a saved context for USER domains	*/
	if (current->id && !current->ctx_level)
		context_save();
	/* Choose the new current domain 		*/
	/* Wait for interrupt if no domain found	*/
	while (!sched->get()) {
		unsigned long cpsr;

		current = idle_domain;
		cpsr = _get_cpsr();
		debsched("cpsr %08lx\n", cpsr);
		wfi();
	}

	task_running = 1;

	debsched("current: %d\n", current->id);

	/* Reset our need to schedule		*/
	sched->need_resched = 0;

	/* Wakeup current domain	*/
	debsched("wakeup at %08lx\n", current->ctx.sregs.pc);
	/* The specific scheduler will take care on interrupt processing */
	if (sched->wake)
		sched->wake(current);
	event_new(EVT_SCHEDIN);
	//context_trace(&current->ctx);
	context_restore();
	switch_to();
}

