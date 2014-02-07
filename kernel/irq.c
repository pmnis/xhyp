/*
 * irq.c
 *
 * Interrupt handling
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

#include <xhyp/config.h>
#include <xhyp/irq.h>
#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/domains.h>
#include <xhyp/hyp.h>
#include <xhyp/debug.h>
#include <xhyp/shared_page.h>
#include <xhyp/event.h>

/** @file irq.c
 * @brief IRQ handling
 */


/** @var NB_IRQ
 * @brief number of entries in the irq_table
 */
#define NB_IRQ	32

/** @var int (*irq_table[NB_IRQ])(int, unsigned long)
 * @brief table of IRQ handlers
 */
int (*irq_table[NB_IRQ])(int, unsigned long);

/* IRQ init has yet not much to do */
void irq_init(void)
{
	pic_init();
}

void wfi(void)
{
	event_new(EVT_WFI);
	sched->need_resched++;
	_wfi();
}

/*
 * This is the hypervisor irq_request routine
 * irq: IRQ BIT provided
 */
int irq_request(int irq, int (*handler)(int, unsigned long))
{
	irq_table[irq] = handler;
	pic_enable(irq);
	return 0;
}

/*
 * Per domain virtual interrupt handling
 * irq: IRQ_MASK
 */
void send_irq(struct domain *d, unsigned long irq)
{
	struct shared_page *s;

	s = d->sp;
	if (!(s->v_irq_enabled & irq))
		return;
	if (d->id == 4 && irq != 0x10) debinfo("IRQ for domain %d : %08lx\n", d->id, irq);
	s->v_irq_pending |= irq;
	switch (d->state) {
	case DSTATE_SLEEP:
		sched->add_from_sleep(d);
	case DSTATE_READY:
		sched->need_resched++;
	}
}
#if 0
int ic = 0;
void send_irq(struct domain *d, unsigned long irq)
{
	struct shared_page *s;

	s = d->sp;
	if (d->state & (DSTATE_STOP|DSTATE_NONE|DSTATE_DEAD))
		return;

	if (d->type == DTYPE_HYP) return;	/* Hypervizor domain	*/

	debirq("d[%d] mode:%d: %08lx\n", d->id, d->mode, irq);
	debirq("trace for %d\n", d->id);

	if (d->mode == DMODE_IRQ) { 		/* Already in IRQ	*/
		debirq("IN IRQ MODE: irq %08lx\n", irq);
		s->v_irq_cached |= irq;		/* Remember irq		*/
		return;
	}
	if (d->id != 4) debirq("IN MODE: %08lx\n", d->mode);
	debirq("enab %08lx mask %08lx cpsr %08lx\n", s->v_irq_enabled, s->v_irq_mask, s->v_cpsr);
	if (!(s->v_irq_enabled & irq)) return;	/* IRQ not enabled	*/
	if ((s->v_irq_mask & irq)) return;	/* IRQ are masked	*/
	if (s->v_cpsr & dis_irqs) return;	/* SOC IRQS disabled	*/
	if (d->id != 4) debirq("d[%d] E:%08lx M:%08lx\n", d->id, s->v_irq_enabled, s->v_irq_mask);

	/* update statistics	*/
	d->irq++;
	/* If the domain is sleeping, wake it up	*/
	debirq("[%d]: state 0x%08lx mode 0x%08lx\n", d->id, d->state, d->mode);
	if (d->state == DSTATE_SLEEP) {
		sched->add(d);
		d->state = DSTATE_READY;
		debirq("[%d]: state 0x%08lx mode 0x%08lx\n", d->id, d->state, d->mode);
	}
	/* mark the IRQ as pending in the shared page	*/
	s = d->sp;
	s->v_irq_pending |= irq;

	if (d->mode == DMODE_ABT) return;	/* do not interrupt abort */
	irq &= ~s->v_irq_mask;			/* clear masked IRQ	*/
	if (!irq) return;			/* deliver later	*/

	/* The domain will transit to handle IRQ		*/
	/* New mode is DMODE_IRQ, It will save ctx if needed	*/
	/* and increment need_resched				*/
	debirq("SET DMODE_IRQ\n");
	mode_new(d, DMODE_IRQ);
	debirq("ID %d\n", d->id);
}
#endif
/*
 * Virtual interruptions for the domains
 */
void do_v_irq(unsigned long num)
{
	struct domain *d;
	int i;

	for (i = 0, d = domain_table; i < nb_domains; i++, d++)
		send_irq(d, (1 << num));
}

/*
 * The main IRQ routine called from asm(IRQ)->do_IRQ
 */
void do_irq(void)
{
	unsigned long mask;
	int irq = 0;

	mask = pic_status();
	/* Put back current for scheduling */
	debirq("mask: %08lx\n", mask);
	while(mask) {
		if (mask & 0x01) {
			if (irq_table[irq]) {
				pic_clear(irq);
				irq_table[irq](irq, NULL);
				do_v_irq(irq);
				pic_enable(irq);
			} else {
				panic(_context, "Spurious Interrupt");
			}
		}
		mask = mask >> 1;
		irq++;
	}
	debirq("END\n");
}


/** fn int hyp_irq_return(void)
 * @brief Hypercall to retrun from interrupt mode
 *
 * @detailed
 * 	called when the guest finished his IRQ handler
 * 	set the context back from the shared page
 * 	where the domain could have changed it
 * 	for context switch purpose for exemple.
 */
int hyp_irq_return(void)
{
	struct shared_page *s = current->sp;

	event_new(EVT_IRQRET);

	s->v_irq_pending &= ~ s->v_irq_mask;

	if (s->v_irq_cached) {
		s->v_irq_pending |= s->v_irq_cached;
		s->v_irq_cached = 0;
	}

	debirq("pending: s->v_irq_pending %08lx\n", s->v_irq_pending);

	debirq("set mode %d\n", current->old_mode);
	/* On Interrupt return we do not save the interrupt context	*/
	/* It will be reset from scratch on next interrupt		*/
	/* But we need to say we did and increment the ctx_level	*/
	current->ctx_level++;
	mode_restore(current);
	context_restore();

	switch_to();
	return 0;
}


/** @fn int hyp_irq_request(void)
 * @brief Registers PC and SP for the IRQ mode
 *
 * @detailed
 * Parameters are passed through registers:
 * - CTX_arg0 is the PC
 * - CTX_arg1 is the SP
 */
int hyp_irq_request(void)
{
	struct shared_page *s = current->sp;

	s->context_irq.sregs.pc = _context->regs.regs[0];
	s->context_irq.sregs.sp = _context->regs.regs[1];
	debirq("PC: %08lx\n", s->context_irq.sregs.pc);
	debirq("SP: %08lx\n", s->context_irq.sregs.sp);
	return 0;
}

/** @fn int hyp_irq_enable(void)
 * @brief enables IRQ depending on R0
 *
 * @detailed
 * If register R0 is 0 then enables IRQ through the
 * shared page virtual processor status: v_cpsr.
 *
 * If register R0 is not 0 then it is a mask of IRQs
 * to be enabled in the v_irq_enabled of the shared page
 *
 * If some IRQ are pending and not masked then schedule
 *
 * The v_irq_mask of the shared_page may be handled
 * directly by the domain. But the domain should not
 * enable IRQs by just setting the v_irq_enabled mask
 * of the shared page directly because it would loose
 * pending interrupts.
 */
extern int deb_on;
int hyp_irq_enable(void)
{
	struct shared_page *s = current->sp;
	unsigned long irq;
	unsigned long in, back;

	in = _context->sregs.spsr;
	irq = _context->regs.regs[0];

	if (irq) {
		s->v_irq_enabled |= irq;
	} else {
		s->v_cpsr &= ~dis_irqs;
	}

	if (s->v_cpsr == 0x12) { /* Should GET AWAY	*/
		//debpanic("SET CPSR to %08lx\n", s->v_cpsr);
		//show_ctx(&current->ctx);
		//exit();
	}

	if (s->v_cpsr & dis_irqs)	/* Interrupt are disabled on VCPU */
		return 0;		/* Just return	*/

	irq = s->v_irq_pending & ~s->v_irq_mask;
	if (irq)			/* If non masked pending Interrupt */
		schedule();		/* schedule will act upon it	*/

	if (sched->need_resched)	/* The timer needs to reschedule */
		schedule();

	back = _context->sregs.spsr;	/* Should GET AWAY     */
	if (in != back) {
		debpanic("Stop\n");
		while(1);
	}
	if (s->v_cpsr == 0x12) {	/* Should GET AWAY     */
		debpanic("STOP\n");
		while(1);
	}
	/* IT not disabled, no IT pending or if then it is masked	*/
	/* Nothing to do any more just return	*/
	return 0;
}

/** @fn int hyp_irq_disable(void)
 * @brief disables IRQ depending on R0
 *
 * @detailed
 * If register R0 is 0 then disable IRQ through the
 * shared page virtual processor status: v_cpsr.
 *
 * If register R0 is not 0 then it is a mask of IRQs
 * to be masked in the v_irq_enabled of the shared page
 *
 * If some IRQ are pending and not masked then send ourself
 * a softirq to schedule in IRQ mode
 */
int hyp_irq_disable(void)
{
	struct shared_page *s = current->sp;
	int irq = _context->regs.regs[0];

	switch (irq) {
	case 0:
		s->v_cpsr |= dis_irqs;
		break;
	default:
		debirq("Enabling %08lx\n", irq);
		s->v_irq_enabled &= ~irq;
		break;
	}
	if (s->v_cpsr == 0x12) {
		debpanic("unprotected IRQ\n");
		while(1);
	}
	return 0;
}

