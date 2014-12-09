/*
 * exceptions.c
 *
 * Exceptions
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

#include <xhyp/stdlib.h>
#include <xhyp/sched.h>
#include <xhyp/irq.h>
#include <xhyp/abort.h>
#include <xhyp/mmu.h>
#include <xhyp/hyp.h>
#include <xhyp/shared_page.h>
#include <xhyp/event.h>

void exp_undefined(void)
{
	struct shared_page *s = current->sp;
	unsigned long *ptr;
	unsigned long instr = _context->sregs.pc;

	ptr = (unsigned long *) dom_virt_to_hyp(current, instr);
	ptr--;
	debpanic("UNDEF INSTR instr %08lx at %08lx cpsr %08lx\n", *ptr, ptr,  _context->cpsr);
	if (!current->id)
		panic(_context, "Cannot recover from Undef Instr");
	debpanic("Undefined instruction at %08lx\n", _context->sregs.pc);
	debpanic("Called from............. %08lx\n", _context->sregs.lr);
	debpanic("SPSR.................... %08lx\n", _context->sregs.spsr);
	debpanic("REAL CPSR............... %08lx\n", _context->cpsr);
	debpanic("Virtual CPSR............ %08lx\n", s->v_cpsr);
	debpanic("Stopping domain %d\n", current->id);
	debpanic("Irqs on domain %d: E:%08lx M:%08lx A:%08lx\n", current->id, s->v_irq_enabled, s->v_irq_mask, s->v_irq_ack);

	sched->kill(current);

	/* NEVER REACHED */
	debpanic("NEVER REACHED\n");
	while(1);
}

/*
	When we receive an interrupt we allways save the user context.
	So it is easier to dispatch interrupt if needed or to switch
	to a new domain.
*/

void show_irqs(void)
{
	int i;
	struct domain *d;
	struct shared_page *s;

	printk("size sp: %d\n", sizeof(*s));
	for (i = 1; i <= nb_usr_domains; i++) {
		d = &domain_table[i];
		s = d->sp;
		printk("domain %d sp: %p v_irq_enabled %08lx v_irq_mask %08lx v_irq_pending %08lx\n", d->id, s, s->v_irq_enabled, s->v_irq_mask, s->v_irq_pending);
	}
}

void exp_irq(void)
{
	debirq("\n");
	event_new(EVT_IRQ);

	debirq("switch from %08lx\n", _context->sregs.pc);
	do_irq();


	debirq("sched->need_resched: %d\n", sched->need_resched);
	if (sched->need_resched) {
		schedule();
	}

	event_new(EVT_IRQOUT);
	debctx("\n");
	debirq("switch to %08lx\n", _context->sregs.pc);
        context_verify();
        switch_to();
}

void exp_fiq(void)
{
	event_new(EVT_FIQ);
	panic(NULL, "Unimplemented");
}


void exp_data_abrt(void)
{
	unsigned long far;
	unsigned long dfsr;
	unsigned long fsr;
	unsigned long *instr;

	debctx("\n");

	event_new(EVT_ABT);

        dfsr = _get_dfsr();
        far = _get_far();

	switch (dfsr & 0x0f) {
	case 0x05:	
		fsr = XHYP_FLT_TRANSLAT;
		break;
	case 0x07:
	case 0x0f:
		fsr = XHYP_FLT_PAGE;
		break;
	case 0x0d:
		fsr = XHYP_FLT_SEC;
		break;
	default:
		debpanic("Unknown DFSR: %08lx\n", dfsr);
		while(1);
	}

	instr = (unsigned long *) dom_virt_to_hyp(current, _context->sregs.pc);
	debabt("Instruction %08lx at %08lx : %08lx\n", _context->sregs.pc, instr, *instr);
	if (!(*instr & (1 << 20))) {
		fsr |= XHYP_FLT_WRITE;
		debabt("XHYP_FLT_WRITE\n");
	}

	if (current->ctx_level) {
		debpanic("current->ctx_level: %d\n", current->ctx_level);
		event_dump_last(20);
		while(1);
	}

	do_abort(far, fsr);

	event_new(EVT_ABTOUT);

	current->no_check = 1;	/* We do not have same registers */
	context_restore();
	switch_to();

	debpanic("Never reached\n");
	while(1);
}

void exp_prefetch(void)
{
	unsigned long far;
	unsigned long dfsr;
	unsigned long fsr;

	debabt("PC at %08lx\n", _context->sregs.pc);

	debctx("\n");
	//context_save();
	event_new(EVT_ABT);

        dfsr = _get_dfsr();
	debabt("DFSR: %08lx\n", dfsr);
        //dfsr = 5;	/* Say it is a page fault */

	fsr = XHYP_FLT_PREFETCH;

	switch (dfsr & 0x0f) {
	case 0x05:	
		fsr = XHYP_FLT_TRANSLAT;
		break;
	case 0x07:
	case 0x0f:
		fsr = XHYP_FLT_PAGE;
		break;
	case 0x0d:
		fsr = XHYP_FLT_SEC;
		break;
	default:
		debpanic("Unknown DFSR: %08lx\n", dfsr);
		while(1);
	}
        far = _context->sregs.pc;

	do_abort(far, fsr);

	event_new(EVT_ABTOUT);

	current->no_check = 1;	/* We do not have same registers */
	context_restore();
	switch_to();

	debpanic("Never reached\n");
	while(1);
}

unsigned long exp_swi(unsigned long *instr)
{
	unsigned long retval;
	unsigned long *ptr;
	unsigned long callnr;

	/* Get syscall		*/
	ptr = (unsigned long *) dom_virt_to_hyp(current, (unsigned long)(--instr));
	callnr = *ptr & 0x00ffffff;


	/* fake undefined expression because Qemu seems to bug */
	if ((_context->cpsr & 0x1F) == 0x1B) {
		debpanic("FAKED UNDEF SWI %d : instr %08lx cpsr %08lx\n", callnr, *ptr,  _context->cpsr);
		exp_undefined();
	}

	current->flags |= DFLAGS_HYPCALL;
	debhyp("callnr: %d\n", callnr);

	current->hypercall = callnr;
	event_new(EVT_SYSIN);
	if (callnr < _HYP_CALLS && hypercall_table[callnr]) {
		current->nb_hypercalls++;
		hypercall_count[callnr]++;
		retval = hypercall_table[callnr]();
	} else {
		debpanic("Bad system call: instr  %08lx\n", instr);
		debpanic("Bad system call: ptr    %08lx\n", ptr);
		debpanic("Bad system call: *ptr   %08lx\n", *ptr);
		debpanic("Bad system call: callnr %08lx\n", callnr);
		retval = -1;
		event_dump_last(20);
		while(1);
	}

	if (sched->need_resched) {
		debctx("sched->need_resched: %d\n", sched->need_resched);
		schedule();
	}

	event_new(EVT_SYSOUT);
	current->ctx.regs.regs[0] = retval;
	_context->regs.regs[0] = retval;
	current->flags &= ~DFLAGS_HYPCALL;
	switch_to();

	return retval;
}

