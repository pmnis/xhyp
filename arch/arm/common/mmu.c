/*
 * mmu.c
 *
 * Memory management
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
#include <xhyp/lowlevel.h>
#include <xhyp/mmu.h>
#include <xhyp/domains.h>
#include <xhyp/mm.h>
#include <xhyp/stdlib.h>
#include <xhyp/debug.h>
#include <xhyp/hyp.h>
#include <xhyp/shared_page.h>
#include <xhyp/errno.h>
#include <xhyp/vpages.h>

/*
 *
 * Mapping functions
 * - Purpose
 *   build sections PGD entries for XHYP
 * - Params
 * idx: index in PGD
 * d: domain used
 * - Return values
 * map_hyp(idx) HYP PGD entry for the PGD index
 * map_io(idx)  IO  PGD entry for the PGD index
 * map_usr(d, idx) USR RW PGD entry, RTOS or GPOS, for domain d for the index
 * map_usr_ro(d, idx) USR RO PGD entry, RTOS, for domain d for the index
 * map_drv(d, idx) DRV RW PGD entry, RTOS or GPOS, for domain d for the index
 */
pgd_t map_hyp(unsigned long idx)
{
	return PTE_SECTION | SEC_RW_RO | (idx << SECTION_SHIFT & 0xfff00000) | PTE_BITS;
}

pgd_t map_io(unsigned long idx)
{
	return PTE_SECTION | SEC_RW_RO | (idx << SECTION_SHIFT & 0xfff00000) | PTE_BITS;
}

pgd_t map_usr(struct domain *d, unsigned long idx)
{
	unsigned long a = d->base_addr;

	a += idx << SECTION_SHIFT;
	a &= 0xfff00000;
	if (d->type == DTYPE_GPOS)
		return a | PTE_SECTION | SEC_RW_RW | ((12) << 5) | PTE_BITS;
	else
		return a | PTE_SECTION | SEC_RW_RW | ((d->id) << 5) | PTE_BITS;
}

pgd_t map_drv(struct domain *d, unsigned long idx)
{
	unsigned long a;

	a = idx << SECTION_SHIFT;
	a &= 0xfff00000;
	return a | PTE_SECTION | SEC_RW_RW | ((d->id) << 5) | PTE_BITS;
}

pgd_t map_usr_ro(struct domain *d, unsigned long idx)
{
	unsigned long a = d->base_addr;

	a += idx << SECTION_SHIFT;
	a &= 0xfff00000;
	return a | PTE_SECTION | SEC_RW_RO | PTE_BITS;
}

unsigned long pmd_val(struct domain *d, unsigned long address)
{
	unsigned long entry;

	entry = address & 0xfff00000;

	return entry;
}

/*
 * PGD entries aka PMD
 */

/*
 * pmd_in_domain:
 * 	Test if the pointed PGD entry is inside current domain
 */
int pmd_in_domain(unsigned long *p)
{
	unsigned long addr = *p & VSEC_ADDR_MASK;

	return paddr_in_domain(current, addr);
}

/*
 * PMD entries aka PTE
 */
/*
 * new_pmd_at: set pmd entries according to
 * 	the PMD pointed by pmd
 * 	the index of the PMD in the PGD
 */
void new_pmd_at(unsigned long pmd, int index)
{
	unsigned long *p;
	unsigned long *q;
	unsigned long i;

	pmd &= ~(PMD_ALIGN - 1);
	debinfo("pmd: %08lx - index %08lx\n", pmd, index);
	p = (unsigned long *)dom_to_hyp(pmd);
	debinfo("src: %p\n", p);
	q = &current->tbl_l2[index * NB_PTE_ENTRIES];
	debinfo("dst: %p\n", q);
	for(i = 0; i < NB_PTE_ENTRIES; i++, q++, p++) {
		*q = 0;
		if (!(*p & VPTE_VALID_BIT))
			continue;
		deb_printf(DEB_INFO, "PTE[%02x] %08lx: %08lx\n", i, p, *p);
		*q = vpte_to_pte(*p);
		deb_printf(DEB_INFO, "PTE[%02x] %08lx: %08lx\n", i, q, *q);
	}
}

/*
 * new_pgd_at setup the real PGD for current
 * 	from the PGD pointed by pgd
 */
void new_pgd_at(unsigned long *pgd)
{
	unsigned long *p;
	unsigned long *q;
	unsigned long i;

	debinfo("pgd %08lx\n", pgd);
	current->v_pgd = (unsigned long) pgd;
	pgd = (unsigned long *) virt_to_phys(current, (unsigned long)pgd);
	debinfo("pgd %08lx\n", pgd);
	p = pgd;
	q = (unsigned long *) current->tbl_l1;

	p++; q++;	/* First entry is XHYP: do not change	*/
	for(i = 1; i < NB_PMD_ENTRIES; i++, q++, p++) {
		*q = 0;
		//debinfo("p[%02d]: %08lx\n", i, *p);
		if (! (*p & VSEC_VALID_BIT))
			continue;
		if (*p & VSEC_TYPE_BIT) {
			*q = vsec_to_sec(*p);
			if (!pmd_in_domain(q)) {
				*q = 0;
			} else {
				deb_printf(DEB_INFO, "pmd[%02x] %08lx: %08lx\n", i, *p, *q);
			}
		} else {
			deb_printf(DEB_INFO, "COARSE[%02x] %08lx: %08lx\n", i, p, *p);
			*q = vpmd_to_pmd(*p, i);
			deb_printf(DEB_INFO, "COARSE[%02x] %08lx: %08lx\n", i, q, *q);
			new_pmd_at(*p, i);
		}
		debinfo("q[%02d]: %08lx\n", i, *q);
	}
	return;
}

/*
 * MMU System calls
 * hyp_switch_mm: setup new PGD from R0
 * hyp_set_pmd: setup one PGD entry
 * hyp_set_pte: setup one PTE entry
 */
int hyp_switch_mm(void)
{
	unsigned long pgd;

	pgd = _context->regs.regs[0];

	/* calculate real PGD address	*/
	deb_printf(DEB_INFO, "PGD 0x%08lx\n", pgd);
	new_pgd_at((unsigned long *)pgd);

	switch_to();

	return 0;
}

/*
int hyp_set_pte_ext(void)
{
	unsigned long pte;
	unsigned long *ptr;
	unsigned long ext;

	ptr = (unsigned long *) _context->regs.regs[0];
	pte = _context->regs.regs[1];
	ext = _context->regs.regs[2];

	deb_printf(DEB_INFO, "pte: %08lx at %08lx ext: %08lx\n", pte, ptr, ext);
while(1);
	deb_printf(DEB_INFO, "pte: %08lx at %08lx\n", pte, ptr);

	*ptr = pte;

	return 0;
}
*/
/*
 * CTX is:
 * r0: pgd
 * r1: address
 * r2: PTE address
 * r3: PTE value
 */

int hyp_set_pmd(void)
{
	unsigned long pgd;
	unsigned long address;
	unsigned long ptr;
	unsigned long pmd;
	unsigned long *p;
	int i;

	pgd = _context->regs.regs[0];
	address = _context->regs.regs[1];
	ptr = _context->regs.regs[2];
	pmd = _context->regs.regs[3];

	debinfo("pgd: %08lx address %08lx ptr %08lx pmd %08lx\n", pgd, address, ptr, pmd);

	/* find pmd entry	*/
	i = (ptr - pgd)/4;
	debinfo("i: %08lx\n", i);
	p = (unsigned long *) current->tbl_l1;
	debinfo("pgd: %08lx\n", p);
	p[i] = vpmd_to_pmd(pmd, i);
	debinfo("pgd[%x]: %08lx\n", i, p[i]);

	show_ventry((unsigned long *)current->tbl_l1, i << 20);

	switch_to();

	return 0;
}

int hyp_set_pte(void)
{
	unsigned long pgd;
	unsigned long address;
	unsigned long ptr;
	unsigned long pte;
	unsigned long *q;
	int	i1,i2;

	pgd = _context->regs.regs[0];
	address = _context->regs.regs[1];
	ptr = _context->regs.regs[2];
	pte = _context->regs.regs[3];


	deb_printf(DEB_INFO, "pgd: %08lx address %08lx pte: %08lx is %08lx\n", pgd, address, ptr, pte);

	if (pgd != current->v_pgd) {
		debinfo("v_pgd: %08lx != %08lx \n", current->v_pgd, pgd);
		return 0;
	}


	i1 = (address >> 20) & 0xfff;
	i2 = (address >> 12) & 0xff;
	//debinfo("i1: %08lx\n", i1);
	//debinfo("i2: %08lx\n", i2);
	
	q = &current->tbl_l2[i1 * NB_PTE_ENTRIES];
	q += i2;
	//debinfo("dst: %p\n", q);
	if ((pte & VPTE_VALID_BIT)) {
		//deb_printf(DEB_INFO, "PTE[%02x] %08lx\n", i2, pte);
		*q = vpte_to_pte(pte);
		//deb_printf(DEB_INFO, "PTE[%02x] %08lx: %08lx\n", i2, q, *q);
	} else {
		debinfo("invalid PTE %08lx set to 0\n", pte);
		*q = 0;
		while(1);
	}
	
	show_ventry((unsigned long *)current->tbl_l1, address);

	switch_to();

	return 0;
}


int hyp_probe(void)
{
	unsigned long retval = _context->regs.regs[0];
	unsigned long addr = _context->regs.regs[1];
	unsigned long size = _context->regs.regs[2];

	addr = virt_to_phys(current, addr);
	retval = virt_to_phys(current, retval);
	if (addr >= current->base_addr && (addr + size) < (current->base_addr + current->size)) {
		memcpy((char *) retval, (char *) addr, size);
		_context->regs.regs[0] = 0;
	} else
		_context->regs.regs[0] = -EFAULT;

	return 0;
}

unsigned long pmd_real_domain(unsigned long vdom)
{
	unsigned long domain;

	if (current->type == DTYPE_GPOS) {
		domain = vdom & 0x03;
		domain = (domain + D_GPOS_DOMAIN) << DOMAIN_SHIFT;
	} else {
		domain = current->id << DOMAIN_SHIFT;
	}
	return domain;
}

unsigned long pmd_real_address(unsigned long vaddr)
{
	return 0xCAFEC0CA;
}

unsigned long pgd_translate(unsigned long vpgd)
{
	unsigned long *pgd = (unsigned long *) vpgd_to_pgd(vpgd);
	unsigned long *src = (unsigned long *) vpgd;
	unsigned long *dst = pgd;
	int i;

	deb_printf(DEB_INFO, "vpgd: %p\n", src);
	deb_printf(DEB_INFO, "pgd : %p\n", dst);

	for (i = addr_to_sec(xhyp->size); i < PGD_SIZE; i++) {
		if (*src & VSEC_VALID_BIT)  {
			*dst++ = vsec_to_sec(*src++);
		}
	}
	/* The first sections belongs to XHYP	*/
	/* But it is always mapped so do nothing */
	/* *pgd = map_hyp(0);			*/

	return (unsigned long )pgd;
}

unsigned long periph_vbase = 0;
void setup_mmu(void)
{
        _setup_mmu(domain_table[0].tbl_l1);
        /* Now we are working with the MMU      */
        /* We need to set all peripherals again */
        periph_base = periph_vbase;
}
void prepare_mmu_flat(void)
{
	int i, r;
	pgd_t *p = (pgd_t *)domain_table[0].tbl_l1;

	for (i = 0; i < MEMORY_SECTIONS; i++,p++) {
		*p = map_hyp(i);
	}
	r = PERIPH_S_BASE;
	periph_vbase = i << SECTION_SHIFT;
	for (i = 0; i < PERIPH_SECTIONS; i++, p++, r++) {
		*p = map_io(r);
	}
	p = (pgd_t *)domain_table[0xfff].tbl_l1;
	*p = map_hyp(0);
}

int paddr_in_domain(struct domain *d, unsigned long addr)
{
	if ( addr < d->base_addr)
		return 0;
	if (addr >= d->base_addr + d->size)
		return 0;
	return 1;
}
