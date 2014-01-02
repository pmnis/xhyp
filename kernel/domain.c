/*
 * domain.c
 *
 * Gestion des domains
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
#include <xhyp/mmu.h>
#include <xhyp/domains.h>
#include <xhyp/lowlevel.h>
#include <xhyp/hyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/list.h>
#include <xhyp/mm.h>
#include <xhyp/debug.h>
#include <xhyp/soc.h>
#include <xhyp/shared_page.h>
#include <xhyp/setup.h>
#include <xhyp/timer.h>
#include <xhyp/event.h>

#include <sys/io.h>
#include <sys/ports.h>

#define DEBUG_STACK
#undef DEBUG_STACK

extern void show_it(void);


void show_ctx(struct context *ctx)
{
	int index;
	unsigned long *p;

	for (index = 0, p = (unsigned long *)ctx; index < 24; index++, p++) {
		printk("%08x ", *p);
		if ( index % 8 == 7 ) printk("\n");
	}

/*
	deb_printf(DEB_CTX, "sp  	%08x\n", ctx->sregs.sp);
	deb_printf(DEB_CTX, "lr  	%08x\n", ctx->sregs.lr);
	deb_printf(DEB_CTX, "pc  	%08x\n", ctx->sregs.pc);
	deb_printf(DEB_CTX, "spsr	%08x\n", ctx->sregs.spsr);
	deb_printf(DEB_CTX, "cpsr	%08x\n", ctx->cpsr);
	deb_printf(DEB_CTX, "far 	%08x\n", ctx->far);
	deb_printf(DEB_CTX, "dfsr	%08x\n", ctx->dfsr);
	deb_printf(DEB_CTX, "ifsr	%08x\n", ctx->ifsr);
	deb_printf(DEB_CTX, "domain	%08x\n", ctx->domain);
	p = (unsigned *) current->tbl_l1;
	for (index = 0; index < 4; index++)
		deb_printf(DEB_CTX, "%08x\n", *p++);
*/
}

void update_page_table(struct domain *d)
{
	unsigned long borne;
	int i;

	i = d->base_addr >> SECTION_SHIFT;
/*
	deb_printf(DEB_INFO, "PGD de %d a %d\n", i, i);
	page_table[i].type = PTABLE_TYPE_PGD;
	page_table[i].domain = d->id;
	i++;
*/
	borne = i + (d->size >> SECTION_SHIFT);
	for (; i < borne; i++) {
		page_table[i].type = PTABLE_TYPE_USR;
		page_table[i].domain = d->id;
	}
}

unsigned long *show_mmu_entry(struct domain *d, unsigned long address)
{
	unsigned long *ptr = (unsigned long *)d->tbl_l1;
	unsigned long entry;

	entry = ptr[pmd_idx(address)];
	printk("%08lx d[%d] L1[%03x] : %08lx\n", address, d->id, pmd_idx(address), entry);
	if (pmd_type(entry) == PTE_COARSE) {
		ptr = d->tbl_l2;
		entry = ptr[pte_idx(address)];
		printk("%08lx d[%d] L2[%03x] : %08lx\n", address, d->id, pte_idx(address), entry);
		entry = (entry & (PAGE_MASK)) | (address & ~PAGE_MASK);
	} else {
		entry = (entry & (SECTION_MASK)) | (address & ~SECTION_MASK);
	}
	return (unsigned long *) entry;
}

void create_map(struct domain *d, unsigned long flags)
{
	int i, j;
	unsigned long borne = addr_to_sec(xhyp->size);
	unsigned long *p = (unsigned long *) d->tbl_l1;
	unsigned long *q;

	/* First 1M is used by XHYP		*/

	q = d->tbl_l2;
	*p++ = (unsigned long )q | PTE_BITS|PTE_COARSE;
	debpte("[%d] p: %08lx : %08lx\n", d->id, p, *p);
	for (i = 0; i < 0x008; i++, q++) {
		*q = (i << 12) | 0xAA2 ;
		debpte("[%d] q: %08lx : %08lx\n", d->id, q, *q);
	}
	p = (unsigned long *) d->tbl_l1;
	p++;
	//*p++ = 0x812;
	q = (unsigned long *)d->tbl_l1;
	debpte("%08lx.......: %08lx\n", q, *q);
	for (i = 1; i < borne; i++, p++) {
		page_table[i].type = PTABLE_TYPE_HYP;
		*p = map_hyp(i);
		//*p = 0;
	}
	/* Map the memory of the guest		*/
	borne += d->size >> SECTION_SHIFT;
	for (j = 0 ; i < borne ; i++, j++, p++) {
		*p = map_usr(d, j);
		page_table[i].type = PTABLE_TYPE_USR;
		page_table[i].domain = d->id;
	}

	/* Memory above GUEST is forbiden	*/
	borne = NB_SECTIONS_ENTRIES;
	for ( ; i < borne; i++, p++)
		*p = 0;

	/* For some guests we map an offset to memory */
	if (d->type == DTYPE_GPOS) {
		p = (unsigned long *) d->tbl_l1 + d->offset + addr_to_sec(xhyp->size);
		for (j = 0 ; j < d->size >> SECTION_SHIFT ; j++, p++) {
			*p = map_usr(d, j);
		}
	}
	if (d->type == DTYPE_DRV) {
		j = addr_to_sec(d->device);
		i = (d->device + d->device_size) >> SECTION_SHIFT;
		p = (unsigned long *) d->tbl_l1 + j;
		for ( ; j < i ; j++, p++) {
			*p = map_drv(d, j);
			//deb_printf(DEB_INFO, "j: %08lx *p: %08lx\n", j, *p);
		}
	}
}

void sp_update(struct shared_page * sp)
{
	sp->jiffies = jiffies;
}

void driver_init(struct domain *d)
{
	struct shared_page *s = d->sp;
	struct queuing_port *qp;
	int i;

	for (i = 0, qp = s->qp; i < (NB_QUEUING_PORT/2); i++, qp++) {
		queuing_port_init(qp, QPORT_IN);
		qp++;
		queuing_port_init(qp, QPORT_OUT);
	}
}

void shared_page_init(struct domain *d)
{
	struct shared_page *s = d->sp;

	if (!d->id)
		return;
	memset(s, 0, sizeof(*s));
	s->magic = XHYP_SP_MAGIC;
	s->end_magic = XHYP_SP_MAGIC2;
	s->cp15_c0 = _get_c0();
	s->cp15_c1 = _get_c1();
	s->v_cpsr = m_svc | dis_irqs;
}

/*
 * function: setup_domain
 * retval  : 0
 * purpose :
 * 	initialize the domain structure for all guest
 * 	having a state by
 * 		- set mode as USR
 * 		- init registers
 * 		- setup MMU with flat translation
 *	 	- add the domain to scheduler
 */
int setup_domains(void)
{
	struct context *ctx;
	struct domain *d;
	int i,j;

	/* Initialize guests domains	*/
	for (i = 1; i < nb_usr_domains + 1; i++) {
		d = &domain_table[i];
		d->id = i;
		if (!d->state)
			continue;
		/* Initialize state and mode	*/
		d->old_mode = DMODE_INIT;
		mode_set(d, DMODE_SVC);
		/* allocate page table	*/
		alloc_page_tables(d);
		/* setup soft page table	*/
		update_page_table(d);
		/* caluculate the guest domain's rights	*/
		d->rights = guest_to_rights(d->type, d->id);
		/* point to guest context	*/
		ctx = &d->ctx;
		/* Initialize the guest registers	*/
		ctx->sregs.pc = d->start_addr;
		/* Create page table mapping	*/
		create_map(d, 0);
		switch (d->type) {
		case DTYPE_GPOS:
			ctx->sregs.pc += (d->offset << SECTION_SHIFT);
			break;
		case DTYPE_DRV:
			break;
		case DTYPE_RTOS:
			break;
		}
		debsched("      pc at %08lx\n", ctx->sregs.pc);
		ctx->sregs.lr = 0;
		ctx->sregs.sp = 0;
		/* Initialize spsr to user mode */
		ctx->sregs.spsr = m_usr;

		/* Clear standard registers	*/
		for (j = 0; j < 13; j++)
			ctx->regs.regs[j] = 0;
		/* On boot, the Linux kernel awaits 		*/
		/* zero               in R0			*/
		/* the processor ID   in R1			*/
		/* a pointer to atags in R2			*/
		ctx->regs.regs[0] = 0x00000000;
		ctx->regs.regs[1] = 0x00000f34;
		ctx->regs.regs[2] = phys_to_virt(d, init_tags(d));

		ctx->cpsr = mode_init;
		/* Initialize the domain's list	*/
		list_init(&d->list);
		/* Add this domain to the scheduler	*/
		sched->start(d);
		/* Setup shared page	*/
		shared_page_init(d);
		event_new(EVT_START);
	}
	current = &domain_table[0];	/* xhyp is the default domain */
	current->rights = 0xffffffff;
	_hyp_ttb = current->tbl_l1;
	_hyp_rights = current->rights;
	debpte("ttb: %08lx rights %08lx\n", _hyp_ttb, _hyp_rights);
	return 0;
}


void mode_save(struct domain *d, int mode)
{
	struct shared_page *s = d->sp;

	//debinfo("save [%d] mode %x\n", d->id, mode);
	//debinfo("[%d] PC %08lx SP %08lx LR %08lx PSR %08lx\n", d->id, d->ctx.sregs.pc, d->ctx.sregs.sp, d->ctx.sregs.lr, d->ctx.sregs.spsr);
	//show_ctx(&d->ctx);
	//debinfo("VCPSR: %08lx\n", s->v_cpsr);
	switch (mode) {
	case DMODE_IRQ:
		s->context_irq = d->ctx;
		break;
	case DMODE_SVC:
		s->context_sys = d->ctx;
		d->v_cpsr = s->v_cpsr;
		break;
	case DMODE_USR:
		s->context_usr = d->ctx;
		break;
	case DMODE_ABT:
		s->context_abt = d->ctx;
		break;
	case DMODE_UND:
		s->context_und = d->ctx;
		break;
	default:
		debpanic("Should not append, mode %d\n", mode);
		while(1);
		break;
	}
}

void mode_set(struct domain *d, int mode)
{
	struct shared_page *s = d->sp;

	d->mode = mode;
	//debinfo("set [%d] mode %x\n", d->id, mode);
	switch (mode) {
	case DMODE_IRQ:
		d->ctx.sregs = s->context_irq.sregs;
		s->v_cpsr = m_irq|dis_irqs;
		break;
	case DMODE_ABT:
		d->ctx.sregs = s->context_abt.sregs;
		if (d->old_mode == DMODE_SVC)
			d->ctx.sregs.sp = s->context_sys.sregs.sp;
		else {
			debinfo("[%d] old mode %x\n", d->id, d->old_mode);
			//d->ctx.sregs.sp = s->context_usr.sregs.sp;
	//C'est Linux qui doit faire cela pas toi !!!!!
			//while(1);
		}
		s->v_cpsr = m_abt|dis_irqs;
		debabt("save context at %08lx\n", &d->ctx);
		break;
	case DMODE_USR:
		d->ctx = s->context_usr;
		s->v_cpsr = m_usr;
		break;
	case DMODE_SVC:
		d->ctx = s->context_sys;
		s->v_cpsr = (m_svc|d->v_cpsr);
		break;
	default:
		debpanic("Should not append, mode %d\n", mode);
		while(1);
		break;
	}
	d->ctx.sregs.spsr &= 0xff;
	d->ctx.sregs.spsr |= 0x10;
	//debinfo("[%d] PC %08lx SP %08lx LR %08lx PSR %08lx\n", d->id, d->ctx.sregs.pc, d->ctx.sregs.sp, d->ctx.sregs.lr, d->ctx.sregs.spsr);
	//debinfo("VCPSR: %08lx\n", s->v_cpsr);
	//show_ctx(&d->ctx);
	sched->need_resched++;
}

void mode_new(struct domain *d, int mode)
{
	mode_save(d, d->mode);
	d->old_mode = d->mode;
	mode_set(d, mode);
}
