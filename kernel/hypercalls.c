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

/** @file hypercalls.c
 * @brief numerous hypercall implementation
 */

int hyp_NONE(void);

/** @var call_entry_t    hypercall_table[_HYP_CALLS]
 * @brief hypercalls table
 */
call_entry_t	hypercall_table[_HYP_CALLS] = {
/*00*/	hyp_syscall,
	hyp_console,
	hyp_yield,
	hyp_trace,
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
	hyp_usr_return,
	hyp_get_tls,
	hyp_cmpxchg,
	hyp_cmpxchg64
};

/** @var int hypercall_count[_HYP_CALLS]
 * @brief statistics on hypercalls
 */
int hypercall_count[_HYP_CALLS];

/** @fn void hyp_mode_set(unsigned long mode)
 * @brief set new mode for hypercalls
 */
void hyp_mode_set(unsigned long mode)
{
	mode_set(current, mode);
	if (!sched->need_resched)
		context_restore();
}

/** @fn void show_hypercall_count(void)
 * @brief show statistics on hypercalls
 */
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
/** @fn int hyp_task_switch(void)
 * @brief hypercall to switch context
 */
int hyp_task_switch(void)
{
	struct context *ctx;

	/* Point to new context			*/
	ctx = (struct context *)(virt_to_phys(current, CTX_arg0));
	/* Setup the next context		*/
	*_context = *ctx;
	return 0;
}

/** @fn int hyp_trace(void)
 * @brief tracing hyper call
 * show the registers and other context entries
 */
int hyp_trace(void)
{
	event_dump_last(1);
	show_ctx(_context);
	return 0;
}

/** @fn int hyp_NONE(void)
 * @brief place holder for unimplemented hypercall
 */
int hyp_NONE(void)
{
	panic(NULL, "Unimplemented");
	return 0;
}


/** @fn int hyp_new_pgd(void)
 * @brief set new pgd
 *
 * @detailed
 * - CTX_arg0 is a pointer to the user's PGD
 */
int hyp_new_pgd(void)
{
	debinfo("pgd: %08lx\n", CTX_arg0);
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

/** @fn int hyp_abt_return(void)
 * @brief return from ABORT mode
 *
 * @detailed
 * The user may have set the old mode register
 * to handle return from data abort
 */
int hyp_abt_return(void)
{
	event_new(EVT_ABTRET);

        current->flags &= ~DFLAGS_HYPCALL;

	current->ctx_level++;	/* We restore from another mode */
	mode_restore(current);

	context_restore();
	switch_to();

	return 0;
}

/** @fn int hyp_pgfault_request(void)
 * @brief Register PC and SP for the abort mode
 *
 * @detailed
 * Parameters are passed through registers:
 * - CTX_arg0 is the PC
 * - CTX_arg1 is the SP
 */
int hyp_pgfault_request(void)
{
	struct shared_page *s = current->sp;

	/* For compatibility: save it in shared_page	*/

	s->context_abt.sregs.pc = _context->regs.regs[0];
	s->context_abt.sregs.sp = _context->regs.regs[1];
	debinfo("PC: %08lx\n", s->context_abt.sregs.pc);
	debinfo("SP: %08lx\n", s->context_abt.sregs.sp);

	return 0;
}

int hyp_setmode(void)
{
	debpanic("UNIMPLEMENTED\n");
	panic(NULL, "UNIMPLEMENTED");
	return 0;
}

/** @fn int hyp_syscall_request(void)
 * @brief Register PC and SP for the syscall mode
 *
 * @detailed
 * Parameters are passed through registers:
 * - CTX_arg0 is the PC
 * - CTX_arg1 is the SP
 */
int hyp_syscall_request(void)
{
	current->ctx_syscall.sregs.pc = _context->regs.regs[0];
	current->ctx_syscall.sregs.sp = _context->regs.regs[1];
	debinfo("PC: %08lx\n", current->ctx_syscall.sregs.pc);
	debinfo("SP: %08lx\n", current->ctx_syscall.sregs.sp);

	return 0;
}

int deb_on = 0;
unsigned long last_spsr = 0;
/** @fn int hyp_syscall(void)
 * @brief Hypercall to implement system calls
 *
 * @detailed
 * The call depends on the system used, if the domain type is GPOS
 * the syscall number is passed through R7 while RTOS use R0
 * - CTX_reg0 is the callnr
 * - CTX_reg7 is the callnr
 *
 * @bug this must change, no reason to change the registers here
 */
int hyp_syscall(void)
{
	unsigned long callnr;
	//struct shared_page *s = current->sp;

	if (current->type == DTYPE_GPOS) {
		callnr = _context->regs.regs[7];
	} else {
		callnr = _context->regs.regs[0];
	}

	last_spsr = _context->sregs.spsr;

	//if (callnr == 3) {
		//show_entry((unsigned long *) current->tbl_l1);
	//}

	debinfo("========= syscall %d (%x)  SPSR   0x%08lx  PC 0x%08lx=======\n", callnr, callnr, _context->sregs.spsr, _context->sregs.pc);
	if (current->mode != DMODE_USR) {
		event_dump_last(20);
		while(1);
		panic(_context, "Wrong mode\n");
	}

	mode_new(current, DMODE_SYSCALL);

	//debinfo("========= syscall %d  SPSR   0x%08lx =======\n", callnr, s->context_usr.sregs.spsr);
	//debinfo("========= syscall %d  SP     0x%08lx =======\n", callnr, current->ctx.sregs.sp);

	current->syscall++;

	current->ctx.regs.regs[0] = callnr;
	current->no_check = 1;  /* We do not have same registers */
	context_restore();
        switch_to();

	return 0;
}

/** @fn int hyp_syscall_return(void)
 * @brief Hypercall to implement return from system calls
 *
 * @detailed
 * The user have changed the registers for the USER mode
 * on returning from system call
 */
int hyp_syscall_return(void)
{
	struct shared_page *s = current->sp;
	unsigned long retval, spsr;
	struct context *ctx = &s->context_usr;

	retval = s->context_usr.regs.regs[0];
	debhyp("retval: %08lx\n", retval);

        event_new(EVT_SYSRET);

	debinfo("========= syscall %d (%x)  SPSR   0x%08lx  PC 0x%08lx=======\n", s->v_syscall, s->v_syscall, ctx->sregs.spsr, ctx->sregs.pc);
	current->ctx_level++;	/* We restore from another mode */
	mode_restore(current);

	spsr = current->ctx.sregs.spsr;

        current->flags &= ~DFLAGS_HYPCALL;

        context_restore();
        switch_to();


	return 0;
}

/** @fn int hyp_yield(void)
 * @brief Hypercall to implement yield
 *
 * @detailed
 * The domain wants to go to idle mode
 */
int hyp_yield(void)
{
	debirq("BEGIN\n");
	sched->yield();
	/* Never reached	*/
	debpanic("PANIC: should not been reached\n");
	while(1);
	return 0;
}

/** @fn int hyp_console(void)
 * @brief output to the console
 *
 * @detailed
 * This hypercall uses two registers:
 * - REG1 hold the size of a string or 0 if a caracter to print is inside R0
 * - REG0 holds the caracter to print if R1 is 0 or a pointer to a string
 *
 * To facilitate debugging, the console print caracters in color
 * according to the domain id red,green,yellow,blue... i.e 30+x
 * from the standard VT220 color set
 *
 * Normaly domains should not use the console hypercall but 
 * should prefer the io_write hypercall to the standard output
 * which would go to a console driver.
 * 
 * using the console is bad because the output is done on the
 * time of the domain but may also overflow to next scheduled
 * domain.
 *
 * @bug this function should disapear if a console driver is present
 */
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
/** fn int hyp_usr_return(void)
 * @brief Hypercall to return to user mode
 * usefull to implement first user thread
 *
 */
int hyp_usr_return(void)
{
        event_new(EVT_USRRET);
        current->flags &= ~DFLAGS_HYPCALL;

	debinfo("\n");
	current->ctx_level++;	/* We restore from another mode */
	mode_set(current, DMODE_USR);

        context_restore();
        switch_to();

	return 0;
}

/** @fn int hyp_cpuctrl(void)
 * @brief report priviledged registers to the domain
 *
 * @detailed
 * The priviledged register reported depends on the
 * R0 value:
 * - 0: CR0 is returned in R0
 * - 1: CR1 is returned in R0
 *
 * @bug this should go to the arch directory
 */
int hyp_cpuctrl(void)
{
	unsigned long cp;
	unsigned long retval;

	deb_printf(DEB_CALL, "BEGIN\n");
	cp = (int) _context->regs.regs[0];
	switch (cp) {
	case 0:
		retval = _get_c0();
		break;
	case 1:
		retval = _get_c1();
		break;
	case 2:
		break;
	}
	deb_printf(DEB_HYP, "END: 0x%08x\n", retval);
	return retval;
}

/** @fn int hyp_set_domain(void)
 * @brief set the domain rights
 *
 * @detailed
 * This is may be used by the domain to change
 * the access right after having modified the pseudo
 * page tables of the partition
 */
int hyp_set_domain(void)
{
	deb_printf(DEB_HYP, "current->rights: %08lx \n", current->rights);
	current->rights = guest_to_rights(current->type, current->id);
	deb_printf(DEB_HYP, "current->rights: %08lx \n", current->rights);
	return 0;
}

/** @fn int hyp_exit(void)
 * @brief This hypercalls aloow a domain to terminate itself
 */
int hyp_exit(void)
{
	deb_printf(DEB_CALL, "BEGIN\n");
	sched->kill(current);
	/* Never reached	*/
	deb_printf(DEB_CALL, "END\n");
	return 0;
}

/** @fn unsigned long hyp_dmesg(struct event *dst, int nb)
 * @brief copy the ring buffer to a different buffer
 * @param dst the event buffer to store the events
 * @nb count of events to dump
 * @return the count of events dumped
 *
 * @detailed this is used by the console tools to 
 * retrive the hypervisor logs
 */
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

/** @fn int hyp_hyp(void)
 * @brief miscalenous hypercalls for the console driver
 *
 * @detailed
 *	- R0 hold the command
 *	- R1 depends on the command, mostly a count
 *	- R2 depends on the command, mostly a pointer
 *
 * The following commands are available:
 *   - HYPCMD_DMESG to dump the hypervisor log
 *   - HYPCMD_EVENTS to dump the events log
 *   - HYPCMD_GET_PLAN to retrieve the ARINC plan
 *   - HYPCMD_SET_PLAN to set the ARINC plan
 *   - HYPCMD_DOM_GET to dump a domain control data
 *   - HYPCMD_DOM_STOP to stop a domain
 *   - HYPCMD_DOM_RESTART to restart a domain
 */
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
		return n;
	case HYPCMD_EVENTS:
		show_hypercall_count();
		return 1;
#ifdef CONFIG_SCHED_ARINC
	case HYPCMD_GET_PLAN:
		{
			struct major_frame *fp;
			fp = (struct major_frame *)virt_to_phys(current, _context->regs.regs[2]);
			memcpy(fp, major, sizeof(*fp));
		}
		return 1;
	case HYPCMD_SET_PLAN:
		{
			struct major_frame *fp;
			fp = (struct major_frame *)virt_to_phys(current, _context->regs.regs[2]);
			memcpy(major, fp, sizeof(*fp));
		}
		return 1;
#endif
	}

	if (n > nb_usr_domains || n == 0) {
		return 0;
	}

	switch(cmd) {
	case HYPCMD_DOM_GET:
		d = (struct domain *) virt_to_phys(current, _context->regs.regs[2]);
		domain_table[n].d_irq_pending = domain_table[n].sp->v_irq_pending;
		domain_table[n].d_irq_mask = domain_table[n].sp->v_irq_mask;
		domain_table[n].d_irq_enabled = domain_table[n].sp->v_irq_enabled;
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
	 	case DSTATE_RUN:
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
		return 0;
	}
}

/** @fn int hyp_event_send(void)
 * @brief Send an event from domain to domain
 *
 * @detailed
 * This hypercalls allows to send a event stored
 * in a unsigned long to a specific domain.
 *   - R0: the domain to send the event to
 *   - R1: the event to be sent
 */
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

/** @fn int hyp_get_timer(void)
 * @brief returns the timer value in register 0
 */
int hyp_get_timer(void)
{
	debinfo("\n");
	return timer_get();
}

int hyp_get_tls(void)
{
	debpanic("Unimplemented\n");
	while(1);
}
int hyp_cmpxchg(void)
{
	debpanic("Unimplemented\n");
	while(1);
}
int hyp_cmpxchg64(void)
{
	debpanic("Unimplemented\n");
	while(1);
}
