/*
 * pages.h
 *
 * Pages definitions
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

#ifndef __PAGES_H
#define __PAGES_H



#define PTE_BITS	0x00000010

#define PTE_COARSE	0x00000001
#define PTE_SECTION	0x00000002
#define PTE_FINE	0x00000003

#define PTE_LARGE	0x00000001
#define PTE_SMALL	0x00000002
#define PTE_TINY	0x00000003

#define PTE_BUFFERED	0x00000004
#define PTE_CACHED	0x00000008

#define PMD_IDX		0xfff00000
#define PTE_IDX		0x000ff000


#define AP_RO_NA	0x00
#define AP_RW_NA	0x01
#define AP_RW_RO	0x02
#define AP_RW_RW	0x03

#define D_NOACCESS	0x00
#define D_CLIENT	0x01
#define D_RESERVED	0x02
#define D_MANAGER	0x03

#define D_HYP		0xffffffff
#define D_GPOS		0x55000001
#define D_GPOS_DOMAIN  12


#define SEC_RW_RO	(AP_RW_RO << 10)
#define SEC_RW_RW	(AP_RW_RW << 10)
#define SEC_AP_MASK	(0x03 << 10)
#define AP_BIT_W	(0x01 << 10)

#define PTE_AP_RW_RW	0x00000ff0
#define PTE_AP_RW_RO	0x00000aa0
#define PTE_AP_RW_NA	0x00000550

#define PTE_HYP		PTE_AP_RW_NA
#define PTE_SYS		PTE_AP_RW_RO
#define PTE_USR		PTE_AP_RW_RW

#define PTE_HYP_MEM	PTE_SMALLPG|PTE_BUFFERED|PTE_CACHED|PTE_HYP
#define PTE_HYP_IO	PTE_SMALLPG|PTE_HYP

#define PTE_SYS_MEM	PTE_SMALLPG|PTE_BUFFERED|PTE_CACHED|PTE_SYS
#define PTE_SYS_IO	PTE_SMALLPG|PTE_SYS

#define PTE_USR_MEM	PTE_SMALLPG|PTE_BUFFERED|PTE_CACHED|PTE_USR

#define REGION_SYS	0x01
#define REGION_IO	0x02

#define DOMAIN_MASK	0xfffffe1f
#define DOMAIN_SHIFT	5
#define ACCESS_MASK	0xfffff3ff

#define SECTION_MASK	0xfff00000
#define SECTION_SIZE	0x00100000
#define SECTION_SHIFT	20

#define PAGE_MASK	0xfffff000
#define PAGE_SIZE	0x00001000
#define PAGE_SHIFT	12

#define NB_SECTIONS_ENTRIES	0x1000
#define NB_PMD_ENTRIES	NB_SECTIONS_ENTRIES
#define NB_PTE_ENTRIES	0x100

#define PMD_SHIFT	20
#define PTE_SHIFT	12

#define PGD_ALIGN       0x4000
#define PMD_ALIGN       0x400

#define PGD_SIZE	NB_SECTIONS_ENTRIES * sizeof(unsigned long)
#define PMD_SIZE	NB_PTE_ENTRIES * sizeof(unsigned long)
#define MAX_MMU_TABLE_SIZE	PGD_SIZE + NB_SECTIONS_ENTRIES * PMD_SIZE

#ifndef __ASSEMBLY__

/*
 * A real address is calculated from virtual address as
 *   32 ......... 20 ........ 12 ..... 0
 *   | cte index  | pte index | offset |
 *   
 */
/* Page Table Entry (pte) for ARM926
 * The Page table is the last indirection table for the MMU
 */
/*
struct pte {
	unsigned int	type:2;
	unsigned int	buffered:1;
	unsigned int	cached:1;
	unsigned int	ap0:2;
	unsigned int	ap1:2;
	unsigned int	ap2:2;
	unsigned int	ap3:2;
	unsigned int	addr:20;
};
*/

/* Coarse Table Entry (cte) for ARM926
 * The Coarse table is the first indirection table for the MMU
 */
/*
struct cte {
	unsigned int	type:2;
	unsigned int	sbz:2;
	unsigned int	mbo:1;
	unsigned int	domain:4;
	unsigned int	sbz2:1;
	unsigned int	addr:22;
};
*/

typedef unsigned long ulong;
typedef unsigned long pgd_t;

#define CTE_DOMAIN(x)	(x<<5)
#define CTE_ADDRESS(x)	((x) & 0xfffffc00)
#define CTE_SMALLPG	0x00000011

struct cte setup_cte(unsigned long r);
struct pte setup_pte(unsigned long r);
struct pte io_remap_page(unsigned long r);

extern int map_page(int domain, unsigned long rpfn, unsigned long vpfn, unsigned long flags);
extern pgd_t map_hyp(unsigned long idx);
extern pgd_t map_io(unsigned long idx);

static inline unsigned long pmd_type(unsigned long pmd)
{
	return (pmd & 0x03);
}
static inline unsigned long pmd_idx(unsigned long pmd)
{
	return (pmd & PMD_IDX) >> PMD_SHIFT;
}

static inline unsigned long pte_type(unsigned long pte)
{
	return (pte & 0x03);
}
static inline unsigned long pte_idx(unsigned long pte)
{
	return (pte & PTE_IDX) >> PTE_SHIFT;
}

static inline unsigned long addr_to_sec(unsigned long addr)
{
	return addr >> SECTION_SHIFT;
}
#define pte_is_section(p) ((p & 0x03) == 0x02)
#endif
#endif
