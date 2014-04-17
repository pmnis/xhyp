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
/** @file domain.c
 * @brief Domain setup and management
 *
 * @detailed
 * This file includes the setup routines and the mode change
 * routines for the domain management.
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

struct context *_context = (struct context *) 0x7000;

#define DEBUG_STACK
#undef DEBUG_STACK

extern void show_it(void);


/** @fn void show_ctx(struct context *ctx)
 *
 * @brief Print the context on the console
 *
 * @detailed
 * The function prints 24 unsigned long from the context on screen
 *
 * @param ctx is a pointer to the context
 */
void show_ctx(struct context *ctx)
{
	int index;
	unsigned long *p;

	debctx("mode: %08lx old_mode: %08lx\n", current->mode, current->old_mode);

	for (index = 0, p = (unsigned long *)ctx; index < (sizeof(*ctx)/sizeof(long)); index++, p++) {
		printk("R%02d %08x\n", index, *p);
		//if ( index % 8 == 7 ) printk("\n");
	}


	debctx("sp  	%08x\n", ctx->sregs.sp);
	debctx("lr  	%08x\n", ctx->sregs.lr);
	debctx("pc  	%08x\n", ctx->sregs.pc);
	debctx("spsr	%08x\n", ctx->sregs.spsr);	// debug
	debctx("cpsr	%08x\n", ctx->cpsr);
	debctx("far 	%08x\n", ctx->far);
	debctx("dfsr	%08x\n", ctx->dfsr);
	debctx("ifsr	%08x\n", ctx->ifsr);
	debctx("domain	%08x\n", ctx->domain);
	p = (unsigned long *) current->tbl_l1;
	for (index = 0; index < 4; index++)
		debctx("%08x\n", *p++);

}

/** @fn unsigned long *show_mmu_entry(struct domain *d, unsigned long address)
 * @brief A debug function to show the MMU entry
 * associated with a physical address
 *
 * @param d is the domain to search the MMU entries for
 * @param address is the physical address
 * @return a pointer to the MMU entry
 */
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

/** @fn void update_page_table(struct domain *d)
 * @brief Updates the page table for the domain
 * @param d is the pointer to the domain
 *
 * The page table is holding informations on the physical
 * memory used by the hypervisor.
 * - the type of page
 * - the identity of the domain owning the page
 * @sa struct page
 */
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

/** @fn void create_map(struct domain *d, unsigned long flags)
 * @brief Create the first mapping of a domain
 * @param d is the domain to work on
 * @param flags defines the type of mapping
 */
void create_map(struct domain *d, unsigned long flags)
{
	int i, j;
	unsigned long borne;
	unsigned long *p = (unsigned long *) d->tbl_l1;
	unsigned long *q = (unsigned long *) d->tbl_l2;

	debpte("Domain[%d] type %d base %08lx size %08lx offset %08lx\n",
			d->id, d->type, d->base_addr, d->size, d->offset);
	/** First First level entry points to the beginning of 2nd level table */
	*p = (unsigned long )q | PTE_BITS|PTE_COARSE;
	debpte("%08lx.......: %08lx\n", p, *p);
	p++;
	/** First 7 pages are used by XHYP and are read-only	*/
	for (i = 0; i < 0x007; i++, q++) {
		*q = (i << 12)|PTE_AP_RW_RO|PTE_SMALL ;
		debpte(".... q: %08lx : %08lx\n", q, *q);
	}
	/** The 8th page hold context information, is RW	*/
	*q = (i << 12)|PTE_AP_RW_RW|PTE_SMALL ;
	debpte("[%d] q: %08lx : %08lx\n", d->id, q, *q);
	q++;

	/** The second mega up to XHYP size are page tables and belong to XHYP */
	borne = addr_to_sec(xhyp->size);
	p = (unsigned long *) d->tbl_l1;
	p++;
	for (i = 1; i < borne; i++, p++) {
		page_table[i].type = PTABLE_TYPE_HYP;
		*p = map_hyp(i);
		debpte("%08lx.......: %08lx\n", p, *p);
	}
	/** Memory above XHYP size belong to guest		*/
	borne += d->size >> SECTION_SHIFT;
	for (j = 0 ; i < borne ; i++, j++, p++) {
		*p = map_usr(d, j);
		debpte("%08lx.......: %08lx\n", p, *p);
		page_table[i].type = PTABLE_TYPE_USR;
		page_table[i].domain = d->id;
	}

	/** Memory above GUEST is forbiden	*/
	borne = NB_SECTIONS_ENTRIES;
	for ( ; i < borne; i++, p++)
		*p = 0;

	/** For GPOS guests we map an offset to memory for logical maping */
	if (d->type == DTYPE_GPOS) {
		p = (unsigned long *) d->tbl_l1 + d->offset + addr_to_sec(xhyp->size);
		for (j = 0 ; j < d->size >> SECTION_SHIFT ; j++, p++) {
			*p = map_usr(d, j);
		}
	}
	/** For DRV guest we map the devices	*/
	if (d->type == DTYPE_DRV) {
		j = addr_to_sec(d->device);
		i = (d->device + d->device_size) >> SECTION_SHIFT;
		p = (unsigned long *) d->tbl_l1 + j;
		for ( ; j < i ; j++, p++) {
			*p = map_drv(d, j);
			debpte("%08lx.......: %08lx\n", p, *p);
		}
	}
}

void sp_update(struct shared_page * sp)
{
	sp->jiffies = jiffies;
}

/** @fn void driver_init(struct domain *d)
 * @brief Initialise the queing ports of a driver
 * @param d is the driver's domain
 */
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

/** @fn void shared_page_init(struct domain *d)
 * @brief Initialise the shared page of a domain
 * @param d is the domain
 *
 * @detailed
 * Each domain as its own shared page.
 * The shared page holds informations to communicate
 * between the hypervisor and the domain.
 *
 * @sa struct shared_page
 */
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

/** @fn int setup_domains(void)
 * @brief initialise all domains
 *
 * @return 0 in case of success the error number otherwise
 *
 * @detailed
 * initialize the domain structure for all guest
 * having a existing state and for each domain
 * 	- set the mode as USR
 * 	- initiase registers
 * 	- setup MMU with flat translation
 * 	- add the domain to scheduler
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
		d->old_mode2 = DMODE_NONE;
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
		d->ctx_level = 1;	/* We start with a restore */
		d->no_check = 1;	/* Do not check first restore */
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
		ctx->sregs.spsr = m_usr; // first init

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

		d->d_cpsr = mode_init;
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


/** @fn void mode_save(struct domain *d, int mode)
 * @brief Setup the shared page context from domain context
 * depending on the mode
 *
 * @param d is the domain to save
 * @param mode is the mode
 *
 * @detailed
 * The mode is one of:
 *  - DMODE_SVC
 *  - DMODE_IRQ
 *  - DMODE_USR
 *  - DMODE_ABT
 *  - DMODE_UND
 *
 *  The domain structure has a context holding scheduling
 *  informations for the scheduled domain.
 *  The shared_page structure has a context for each mode
 *  and the OS in the partition can use this information
 *  to retrieve the saved context on mode change, like on interrupt.
 *
 *  The struct domain has an element d_cpsr holding the virtual
 *  processor status. On mode_save, the shared_page v_cpsr is
 *  set to the value saved in d_cpsr.
 */
void mode_save(struct domain *d, int mode)
{
	struct shared_page *s = d->sp;

        if (d == current && !d->ctx_level) {
                /* Setup context for next slice         */
                debctx("CTX\n");
                context_save();
        }

	d->d_cpsr = s->v_cpsr;
	switch (mode) {
	case DMODE_IRQ:
		s->context_irq = d->ctx;
		break;
	case DMODE_SVC:
		s->context_sys = d->ctx;
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

/** @fn void mode_ret(struct domain *d, int mode)
 * @brief Setup the domain context from shared_page context
 * depending on the mode when returning from exception
 *
 * @param d is the domain to save
 * @param mode is the mode
 *
 * @detailed
 * The mode is one of:
 *  - DMODE_SVC
 *  - DMODE_IRQ
 *  - DMODE_USR
 *  - DMODE_ABT
 *  - DMODE_UND
 *
 *  The domain structure has a context holding scheduling
 *  informations for the scheduled domain.
 *  The shared_page structure has a context for each mode
 *  and the OS in the partition can set this information
 *  to modify saved context on mode change, like on interrupt.
 *
 *  The struct domain has an element d_cpsr holding the virtual
 *  processor status. On mode_save, the domain d_cpsr is
 *  set to the value saved in struct shared_page v_cpsr.
 */
void mode_restore(struct domain *d)
{
	struct shared_page *s = d->sp;
	int mode;

	mode = d->mode;
	d->mode = d->old_mode;
	d->old_mode = d->old_mode2;
	d->old_mode2 = DMODE_NONE;

	s->v_cpsr = s->v_spsr;

	switch (d->mode) {
	case DMODE_USR:
		d->ctx = s->context_usr;
		break;
	case DMODE_SVC:
		d->ctx = s->context_sys;
		s->v_spsr = d->d_spsr;
		break;
	default:
		debpanic("Should not append, from mode %d to mode %d\n", mode, d->mode);
		while(1);
		break;
	}
	d->ctx.sregs.spsr &= ~mask_domain;
	d->ctx.sregs.spsr |= mode_domain;
	d->d_sum = context_sum(&d->ctx);

	d->flags &= ~DFLAGS_HYPCALL;
}


/** @fn void mode_set(struct domain *d, int mode)
 * @brief Setup the domain context from shared_page context
 * depending on the mode
 *
 * @param d is the domain to save
 * @param mode is the mode
 *
 * @detailed
 * The mode is one of:
 *  - DMODE_SVC
 *  - DMODE_IRQ
 *  - DMODE_USR
 *  - DMODE_ABT
 *  - DMODE_UND
 *
 *  The domain structure has a context holding scheduling
 *  informations for the scheduled domain.
 *  The shared_page structure has a context for each mode
 *  and the OS in the partition can set this information
 *  to modify saved context on mode change, like on interrupt.
 *
 *  The struct domain has an element d_cpsr holding the virtual
 *  processor status. On mode_save, the domain d_cpsr is
 *  set to the value saved in struct shared_page v_cpsr.
 */
void mode_set(struct domain *d, int mode)
{
	struct shared_page *s = d->sp;

	d->old_mode2 = d->old_mode;
	d->old_mode = d->mode;
	d->mode = mode;
	/* Save SPSR to be used by domains to know old mode	*/
	d->d_spsr = s->v_spsr;
	s->v_spsr = s->v_cpsr;

	switch (mode) {
	case DMODE_IRQ:
		d->ctx.sregs = s->context_irq.sregs;
		s->v_cpsr = (m_irq|dis_irqs);
		break;
	case DMODE_ABT:
		d->ctx.sregs = s->context_abt.sregs;
		s->v_cpsr = m_abt|dis_irqs;
		break;
	case DMODE_USR:
		d->ctx = s->context_usr;
		s->v_cpsr = m_usr;
		break;
	case DMODE_SVC:
		d->ctx = s->context_sys;
		s->v_cpsr = (m_svc|dis_irqs);
		break;
	case DMODE_SYSCALL:
		d->ctx = s->context_usr;
		d->ctx.sregs = d->ctx_syscall.sregs;	/* Only special registers are updated */
		d->mode = DMODE_SVC;
		s->v_cpsr = (m_svc|dis_irqs);
		break;
	default:
		debpanic("Should not append, mode %d\n", mode);
		while(1);
		break;
	}
	d->ctx.sregs.spsr &= ~mask_domain;
	d->ctx.sregs.spsr |= mode_domain;
	d->d_sum = context_sum(&d->ctx);
	d->flags &= ~DFLAGS_HYPCALL;
}

/** @fn void mode_new(struct domain *d, int mode)
 * @brief The function is called on changing mode to
 * save the old context and set a new one.
 *
 * @param d is the domain
 * @param mode is the new mode
 *
 * @detailed
 * The old mode of the domain is set to the actual mode of the domain.
 * The mode of the domain is set to the value of the mode parameter.
 */
void mode_new(struct domain *d, int mode)
{
	mode_save(d, d->mode);
	mode_set(d, mode);
}

