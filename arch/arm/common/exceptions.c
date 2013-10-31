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

	if (!current->id)
		panic(_context, "Cannot recover from Undef Instr");
	debpanic("Undefined instruction at %08lx\n", _context->sregs.pc);
	debpanic("Called from............. %08lx\n", _context->sregs.lr);
	debpanic("CPSR.................... %08lx\n", s->v_cpsr);
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
	current->ctx = *_context;

	debirq("switch from %08lx\n", _context->sregs.pc);
	do_irq();
	debirq("switch to %08lx\n", _context->sregs.pc);
        context_verify(_context);
        switch_to();
}

void exp_fiq(void)
{
	panic(NULL, "Unimplemented");
}


void exp_data_abrt(void)
{
	struct shared_page *s = current->sp;
	unsigned long far;
	unsigned long dfsr;

	event_new(EVT_ABT);

	s->context_abt = *_context;
	current->sr_abt = s->v_cpsr;
        dfsr = _get_dfsr();
        far = _get_far();

	do_abort(far, dfsr);

        context_verify(_context);
        switch_to();
}

void exp_prefetch(void)
{
	struct shared_page *s = current->sp;
	unsigned long far;
	unsigned long dfsr;

	debabt("PC at %08lx\n", _context->sregs.pc);
	event_new(EVT_ABT);

	s->context_abt = *_context;
	current->sr_abt = s->v_cpsr;
        dfsr = 5;	/* Say it is a page fault */
        far = _context->sregs.pc;

	do_abort(far, dfsr);


	debabt("no abt handling\n");
        context_verify(_context);
        switch_to();
}

unsigned long exp_swi(unsigned long *instr)
{
	unsigned long retval;
	unsigned long *ptr;
	unsigned long callnr;

	/* Get syscall		*/
	//ptr = (unsigned long *) virt_to_phys(current, (unsigned long)(--instr));
	ptr = (unsigned long *) dom_virt_to_hyp(current, (unsigned long)(--instr));
	callnr = *ptr & 0x00ffffff;

	/* fake undefined expression because Qemu seems to bug */
	if ((_context->cpsr & 0x1F) == 0x1B) {
		debhyp("FAKED UNDEF SWI %d : %l\n", callnr, _context->cpsr);
		exp_undefined();
	}

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
		while(1);
	}
	debhyp("call[%d] returning to %08lx\n", callnr, _context->sregs.pc);
	event_new(EVT_SYSOUT);
	_context->regs.regs[0] = retval;
	switch_to();

	return retval;
}

