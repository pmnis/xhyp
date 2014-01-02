/*
 * hypercalls.c
 *
 * Gestion des hypercalls
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

#include <xhyp/mmu.h>
#include <xhyp/timer.h>
#include <xhyp/domains.h>
#include <xhyp/shared_page.h>
#include <xhyp/hyp.h>
#include <xhyp/debug.h>
#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/soc.h>
#include <xhyp/event.h>
#include <xhyp/ring.h>
#include <xhyp/abort.h>
#include <xhyp/irq.h>
#include <xhyp/mm.h>
#include <xhyp/serial.h>

#ifdef CONFIG_SCHED_ARINC
#include <xhyp/arinc.h>
#endif

int hyp_NONE(void);


call_entry_t	hypercall_table[_HYP_CALLS] = {
/*00*/	hyp_syscall,
	hyp_console,
	hyp_yield,
	hyp_NONE,
	hyp_NONE,
/*05*/	hyp_irq_request,
	hyp_irq_enable,
	hyp_irq_disable,
	hyp_irq_return,
	hyp_exit,
/*10*/	hyp_cpuctrl,
	hyp_syscall_request,
	hyp_syscall_return,
	hyp_pgfault_request,
	hyp_undef_request,
/*15*/	hyp_enable_mmu,
	hyp_setmode,
	hyp_set_domain,
	hyp_disable_cache,
	hyp_reset,
/*20*/	hyp_idle,
	hyp_flush_cache,
	hyp_flush_icache,
	hyp_flush_user_cache_range,
	hyp_coherent_user_rang,
/*25*/	hyp_flush_kern_dcache_area,
	hyp_dma_inv_range,
	hyp_dma_clean_range,
	hyp_dma_flush_range,
	hyp_dchache_clean_area,
/*30*/	hyp_NONE,
	hyp_set_pte,
	hyp_tlb,
	hyp_enable_cache,
	hyp_probe,
/*35*/	hyp_task_switch,
	hyp_copy_page,
	hyp_clear_page,
	hyp_new_pgd,
	hyp_set_pmd,
/*40*/	hyp_NONE,
	hyp_NONE,
	hyp_get_timer,
	hyp_io_open,
	hyp_io_read,
/*45*/	hyp_io_write,
	hyp_io_ioctl,
	hyp_io_close,
	hyp_event_send,
	hyp_preempt_disable,
/*50*/	hyp_preempt_enable,
	hyp_hyp,
	hyp_abt_return,
	hyp_usr_return
};

int hypercall_count[_HYP_CALLS];

void show_hypercall_count(void)
{
	int i;

	for (i = 0; i < _HYP_CALLS; i++) {
		fprintf(0, "%5d ", hypercall_count[i]);
		if (!((i+1) % 10)) fprintf(0, "\n");
	}
	fprintf(0, "\n");
}

void show_mmu(unsigned long *pgd)
{
	int i;
	fprintf(0, "pgd at 0x%08x\n", pgd);
	for (i=0 ; i < 4096; i++) {
		fprintf(0, "%04d : %08x : \n", i, pgd[i]);
		if ((pgd[i] & 0x03) == 0x02)
			show_section(pgd + i);
	}
}

/*
 * hyp_task_switch
 * r0: saved R0
 * r1: CPSR
 * r2: PC
 * r3: 
 */

int context_verify(struct context *ctx)
{
	/* force SPSR to user mode	*/
	ctx->sregs.spsr &= ~mask_domain;
	ctx->sregs.spsr |= mode_domain;
	return 0;
}

int hyp_task_switch(void)
{
	struct shared_page *s = current->sp;
	struct context *ctx;

	/* save old context to shared page	*/
	//s->context_sys = *_context;
	/* Point to new context			*/
	ctx = (struct context *)(virt_to_phys(current, CTX_arg0));
{
	unsigned long *p = (unsigned long *)ctx;
	int i;

	for (i = 0; i < 32; i++, p++) {
		debhyp("%02d %p %08lx\n", i, p, *p);
	}
}
	ctx = &s->context_sys;
	//ctx->sregs.lr = ctx->sregs.pc;
	/* Verify context				*/
	context_verify(ctx);
	/* Copy new context to current context	*/
	*_context = *ctx;
	//debinfo("PC: %08lx\n", ctx->sregs.pc);
	return 0;
}

int hyp_NONE(void)
{
	panic(NULL, "Unimplemented");
	return 0;
}


int hyp_new_pgd(void)
{
	new_pgd_at((unsigned long *) CTX_arg0);
	switch_to();
	return 0;
}


/*
 * unsigned long hyp_enable_mmu(void)
 * PGD size is twice the normal size of the ARM9 PGD
 * 	The second part is used to hold the real PGD
 * 	while the first part is used for the guest view of the PGD
 * r0: pgd
 * r1: rights
 * r2: PC after we enabled the MMU
 */

int hyp_enable_mmu(void)
{
	debpanic("UNIMPLEMENTED\n");
	panic(NULL, "UNIMPLEMENTED");
	return 0;
}

int hyp_undef_request(void)
{

	debpanic("UNIMPLEMENTED\n");
	panic(NULL, "UNIMPLEMENTED");
	return 0;
}

int hyp_abt_return(void)
{
	struct shared_page *s = current->sp;

	event_new(EVT_ABTRET);


	debinfo("SET CPSR to %08lx\n", s->v_cpsr);

	mode_set(current, current->old_mode);
	event_new(EVT_ABTOUT);

	*_context = current->ctx;

	context_verify(_context);

	/* be sure we do not enable a priviledge mode */
	_context->sregs.spsr &= ~0xef;
	switch_to();

	debpanic("NEVER REACHED\n");
	return 0;
}
int hyp_pgfault_request(void)
{
	struct shared_page *s = current->sp;

	/* For compatibility: save it in shared_page	*/

	s->context_abt.sregs.pc = _context->regs.regs[0];
	s->context_abt.sregs.sp = _context->regs.regs[1];
	debabt("PC: %08lx\n", s->context_abt.sregs.pc);
	debabt("SP: %08lx\n", s->context_abt.sregs.sp);

	return 0;
}

int hyp_setmode(void)
{
	debpanic("UNIMPLEMENTED\n");
	panic(NULL, "UNIMPLEMENTED");
	return 0;
}

int hyp_syscall_request(void)
{
	struct shared_page *s = current->sp;

	s->context_sys.sregs.pc = _context->regs.regs[0];
	s->context_sys.sregs.sp = _context->regs.regs[1];
	return 0;
}

int hyp_syscall(void)
{
	unsigned long callnr;

	if (current->type == DTYPE_GPOS) {
		callnr = _context->regs.regs[7];
	} else {
		callnr = _context->regs.regs[0];
	}
	debinfo("========= syscall %d  -  0x%08lx============\n", callnr, callnr);
	if (current->mode != DMODE_USR) panic(_context, "Wrong mode\n");
while(1);
	mode_new(current, DMODE_SVC);

	*_context = current->ctx;

	_context->regs.regs[0] = callnr;

	current->syscall++;

	switch_to();
while(1);
	/* Never reached	*/
	return 0;
}

int hyp_syscall_return(void)
{

	debinfo("\n");

	/* fake shadow registers save */
	mode_set(current, DMODE_USR);

	switch_to();
	
	debpanic("NON IMPLEMENTE\n");
while(1);
	/* Unreached	*/
	return 0;
}

int hyp_yield(void)
{
	debinfo("BEGIN\n");
	sched->yield();
	/* Never reached	*/
	deb_printf(DEB_CALL, "END\n");
	return 0;
}

#define COLOR_BLACK     "[30m"
#define COLOR   "[3"

int hyp_console(void)
{
	char *s;
	int n;
	char mini_buffer[2];
	char colors[2];

	n = (int) _context->regs.regs[1];

	if (n == 0) {
		n = 1;
		mini_buffer[1] = 0;
		mini_buffer[0] = (unsigned char) (_context->regs.regs[0] & 0xFF);
		s = mini_buffer;
	} else {
		s = (char *) (virt_to_phys(current, _context->regs.regs[0]));
	}
	colors[0] = '0' + current->id;
	colors[1] = 'm';
	serial_write(COLOR, 3);
	serial_write(colors, 2);
	serial_write(s, n);
	serial_write(COLOR_BLACK, 5);
	return n - 1;
}

/*
 * function: hyp_irq_return
 * purpose :
 * 	called when the guest finished his IRQ handler
 * 	set the context back from the shared page
 * 	where the domain could have changed it
 * 	for context switch purpose for exemple.
 */
int hyp_irq_return(void)
{
	event_new(EVT_IRQOUT);
	mode_set(current, current->old_mode);

	/* be sure we do not enable a priviledge mode */
	context_verify(_context);

	event_new(EVT_IRQRET);

	return 0;
}

int hyp_usr_return(void)
{
	mode_set(current, DMODE_USR);

	schedule();

	return 0;
}


int hyp_irq_request(void)
{
	struct shared_page *s = current->sp;
	s->context_irq.sregs.pc = _context->regs.regs[0];
	s->context_irq.sregs.sp = _context->regs.regs[1];
	//debinfo("PC: %08lx SP: %08lx\n", s->context_irq.sregs.pc, s->context_irq.sregs.sp);
	return 0;
}

int hyp_irq_enable(void)
{
	struct shared_page *s = current->sp;
	unsigned long irq = _context->regs.regs[0];

	if (irq) {
		//debinfo("Enabling %08lx\n", irq);
		s->v_irq_enabled |= irq;
	} else
		s->v_cpsr &= ~dis_irqs;
	//debinfo("SET CPSR to %08lx\n", s->v_cpsr);
	if (s->v_cpsr == 0x12)
		while(1);

	if (s->v_cpsr & dis_irqs)
		return 0;

	irq = s->v_irq_enabled & ~s->v_irq_mask;
	if (irq)
		send_irq(current, irq);
	//debinfo("back with cpsr: %08lx\n", s->v_cpsr);
	//debinfo("back with pc  : %08lx\n", _context->sregs.pc);
	//debinfo("back with sp  : %08lx\n", _context->sregs.sp);
	if (s->v_cpsr == 0x12) {
		debinfo("STOP\n");
		while(1);
	}
	return 0;
}

int hyp_irq_disable(void)
{
	struct shared_page *s = current->sp;
	int irq = _context->regs.regs[0];

	switch (irq) {
	case 0:
		s->v_cpsr |= dis_irqs;
		break;
	default:
		s->v_irq_enabled &= ~irq;
		break;
	}
	if (s->v_cpsr == 0xd3)
		return 0;
	debirq("SET CPSR to %08lx\n", s->v_cpsr);
	//if (s->v_cpsr == 0xd2)
		//debug_level = DEB_ALL;
	if (s->v_cpsr == 0x12) {
		debpanic("unprotected IRQ\n");
		while(1);
	}
	return 0;
}

int hyp_cpuctrl(void)
{
	unsigned long cp;

	deb_printf(DEB_CALL, "BEGIN\n");
	cp = (int) _context->regs.regs[0];
	switch (cp) {
	case 0:
		_context->regs.regs[0] = _get_c0();
		break;
	case 1:
		_context->regs.regs[0] = _get_c1();
		break;
	case 2:
		break;
	}
	deb_printf(DEB_HYP, "END: 0x%08x\n", _context->regs.regs[0]);
	return 1;
}

int hyp_set_domain(void)
{
	deb_printf(DEB_HYP, "current->rights: %08lx \n", current->rights);
	current->rights = guest_to_rights(current->type, current->id);
	deb_printf(DEB_HYP, "current->rights: %08lx \n", current->rights);
	return 0;
}

int hyp_exit(void)
{
	deb_printf(DEB_CALL, "BEGIN\n");
	sched->kill(current);
	/* Never reached	*/
	deb_printf(DEB_CALL, "END\n");
	return 0;
}


unsigned long hyp_dmesg(struct event *dst, int nb)
{
	struct event event;
	int n = 0;

	while (ring_get(&trace_ring, &event, sizeof(event)) && nb--) {
		memcpy(dst, &event, sizeof(event));
		n++;
		dst++;
	}
	return n;
}

int hyp_hyp(void)
{
	unsigned long n;
	unsigned long cmd;
	struct domain *d;

	deb_printf(DEB_CALL, "BEGIN\n");
	cmd = (int) _context->regs.regs[0];
	n = (int) _context->regs.regs[1];
	switch(cmd) {
	case HYPCMD_DMESG:
		n= hyp_dmesg((void *)virt_to_phys(current, _context->regs.regs[2]), n);
		_context->regs.regs[0] = n;
		return n;
	case HYPCMD_EVENTS:
		show_hypercall_count();
		_context->regs.regs[0] = 1;
		return 1;
#ifdef CONFIG_SCHED_ARINC
	case HYPCMD_GET_PLAN:
		{
			struct major_frame *fp;
			fp = (struct major_frame *)virt_to_phys(current, _context->regs.regs[2]);
			memcpy(fp, major, sizeof(*fp));
		}
		_context->regs.regs[0] = 1;
		return 1;
	case HYPCMD_SET_PLAN:
		{
			struct major_frame *fp;
			fp = (struct major_frame *)virt_to_phys(current, _context->regs.regs[2]);
			memcpy(major, fp, sizeof(*fp));
		}
		_context->regs.regs[0] = 1;
		return 1;
#endif
	}

	if (n > nb_usr_domains || n == 0) {
		_context->regs.regs[0] = 0;
		return 0;
	}

	_context->regs.regs[0] = 1;
	switch(cmd) {
	case HYPCMD_DOM_GET:
		d = (struct domain *) virt_to_phys(current, _context->regs.regs[2]);
		domain_table[n].v_irq_pending = domain_table[n].sp->v_irq_pending;
		domain_table[n].v_irq_mask = domain_table[n].sp->v_irq_mask;
		domain_table[n].v_irq_enabled = domain_table[n].sp->v_irq_enabled;
		memcpy(d, &domain_table[n], sizeof(*d));
		return 1;
	case HYPCMD_DOM_STOP:
		d = &domain_table[n];
		if (d->state == DSTATE_STOP) return 0;
		d->old_state = d->state;
		d->state = DSTATE_STOP;
		sched->delete(d);
		return 1;
	case HYPCMD_DOM_RESTART:
		d = &domain_table[n];
		//debinfo("%d: old %d state %d\n", d->id, d->old_state, d->state);
		switch (d->state) {
	 	case DSTATE_READY:
	 	case DSTATE_RUN:
	 	case DSTATE_SLEEP:
			return 0;
		}
		switch (d->old_state) {
	 	case DSTATE_SLEEP:
			debinfo("was sleeping\n");
			d->state = DSTATE_SLEEP;
			break;
	 	case DSTATE_STOP:
	 	case DSTATE_NONE:
			//debinfo("was stopped : %d %d\n", DSTATE_READY, DSTATE_STOP);
			d->state = DSTATE_READY;
			break;
		default:
			debinfo("was %d\n", d->old_state);
			break;
		}
		if (d->state == DSTATE_READY)
			sched->add(d);
		return 1;
	default:
		_context->regs.regs[0] = 0;
		return 0;
	}
}

int hyp_event_send(void)
{
	struct shared_page *s;
	int id = _context->regs.regs[0];
	unsigned long event = _context->regs.regs[1];

	if (id < 1 || id >= nb_domains)
		return -1;

	s = domain_table[id].sp;
	s->sampling_port = event;

	send_irq(&domain_table[id],IRQ_MASK_EVENT);

	return 0;
}

int hyp_get_timer(void)
{
	debinfo("\n");
	_context->regs.regs[0] = timer_get();
	return 0;
}

