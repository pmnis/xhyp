/*
 * vpages-utils.h
 *
 * Virtual Pages utilities
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

#ifndef __VPAGES_UTILS_H
#define __VPAGES_UTILS_H

#ifndef __ASSEMBLY__

#include <xhyp/domains.h>
extern unsigned long pt_level1;

static inline unsigned long dom_to_hyp(unsigned long p)
{
	if (p < XHYP_MEM_SIZE)
		return p;
	return p + current->base_addr - XHYP_MEM_SIZE;
}
static inline unsigned long virt_to_phys(struct domain *d, unsigned long p)
{
	if (p < XHYP_MEM_SIZE)
		return p;
	return p + d->base_addr - XHYP_MEM_SIZE - (d->offset << SECTION_SHIFT);
}
static inline unsigned long dom_virt_to_hyp(struct domain *d, unsigned long address)
{
	unsigned long *ptr = (unsigned long *)d->tbl_l1;
	unsigned long entry;

	entry = ptr[pmd_idx(address)];
	if (pmd_type(entry) == PTE_COARSE) {
		ptr = d->tbl_l2;
		entry = ptr[pte_idx(address)];
		entry = (entry & (PAGE_MASK)) | (address & ~PAGE_MASK);
	} else {
		entry = (entry & (SECTION_MASK)) | (address & ~SECTION_MASK);
	}
	return entry;
}

static inline unsigned long phys_to_virt(struct domain *d, unsigned long p)
{
	if (p < XHYP_MEM_SIZE)
		return p;
	return p - d->base_addr + XHYP_MEM_SIZE + (d->offset << SECTION_SHIFT);
}

extern unsigned long pmd_real_domain(unsigned long);

static inline unsigned long vpte_to_pte(unsigned long vpte)
{
	unsigned long pte = dom_to_hyp(VPTE_ADDR_MASK & vpte);

	pte |= PTE_BITS;

	if (vpte & VPTE_VALID_BIT) pte |= PTE_SMALL;
	if (vpte & VPTE_CACHE_BIT) pte |= PTE_CACHED;
	if (vpte & VPTE_BUFFER_BIT) pte |= PTE_BUFFERED;
	if (vpte & VPTE_WRITE_BIT) pte |= PTE_USR;
	else pte |= PTE_SYS;

	return pte;
}

static inline unsigned long vpmd_to_pmd(unsigned long vpmd, int index)
{
	unsigned long pmd;

	pmd = (unsigned long )(&current->tbl_l2[index * 256]);

	pmd |= PTE_COARSE;
	pmd |= pmd_real_domain((vpmd & VPMD_DOMAIN_MASK) >> VPMD_DOMAIN_SHIFT);

	return pmd;
}

#define sec_real_domain		pmd_real_domain
#include <xhyp/debug.h>
static inline unsigned long vsec_to_sec(unsigned long vsec)
{
	unsigned long sec = virt_to_phys(current, vsec & VSEC_ADDR_MASK);

	sec |= PTE_BITS;
	sec |= sec_real_domain((vsec & VSEC_DOMAIN_MASK) >> VSEC_DOMAIN_SHIFT);

	if (vsec & VSEC_VALID_BIT) sec |= PTE_SECTION;
	if (vsec & VSEC_CACHE_BIT) sec |= PTE_CACHED;
	if (vsec & VSEC_BUFFER_BIT) sec |= PTE_BUFFERED;
	if (vsec & VSEC_WRITE_BIT) sec |= SEC_RW_RW;
	else sec |= SEC_RW_RO;
	return sec;
}

static inline unsigned long vpgd_to_pgd(unsigned long vpgd)
{
	unsigned long pgd = vpgd + VPMD_TO_PMD_OFFSET;

	return pgd;
}


#endif /* __ASSEMBLY__ */
#endif
