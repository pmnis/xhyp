/*
 * mmu.h
 *
 * Memory Management Unit definitions
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

#ifndef __MMU_H
#define __MMU_H

#include <xhyp/config.h>
#include <xhyp/pages.h>
#include <xhyp/vpages.h>
#include <xhyp/vpages-utils.h>

#define MEMORY_PAGES (MEMORY_SIZE >> PAGE_SHIFT)
#define PERIPH_PAGES (PERIPH_SIZE >> PAGE_SHIFT)

/* Sections definitions	*/
#define SECTION_SHIFT	20
#define MEMORY_SECTIONS	(MEMORY_SIZE >> SECTION_SHIFT)
#define PERIPH_SECTIONS	(PERIPH_SIZE >> SECTION_SHIFT)
#define PERIPH_S_BASE (PERIPH_BASE >> SECTION_SHIFT)

/* Coarse table will include memory and peripherals     */
#define CT_MEMORY_INDEX ((MEMORY_PAGES / 256))
#define CT_PERIPH_INDEX ((PERIPH_PAGES / 256))

#define PERIPH_VBASE    (CT_MEMORY_INDEX << 20)
#define CT_SIZE  (CT_MEMORY_INDEX + CT_PERIPH_INDEX)
#define PTE_PER_CTE 256
#define PT_SIZE PTE_PER_CTE * CT_SIZE   /* This is fixed by ARM926EJS   */ 


#ifndef __ASSEMBLY__


extern void prepare_mmu_flat(void);
extern void setup_mmu(void);
extern void _flush_tlb(void);
/* Debug stuff */
#ifdef DEBUG
extern void show_section(unsigned long *s);
extern void show_pgd(unsigned long *pgd);
extern int show_level1(unsigned long *coarse);
extern int show_entry(unsigned long *coarse);
extern void show_ventry(unsigned long *pgd, unsigned long vaddr);
#else
static inline void show_section(unsigned long *s){}
static inline void show_pgd(unsigned long *pgd){}
static inline int show_level1(unsigned long *coarse){return 0;}
static inline int show_entry(unsigned long *coarse){return 0;}
static inline void show_ventry(unsigned long *pgd, unsigned long vaddr){}
#endif
extern unsigned long *show_mmu_entry(struct domain *d, unsigned long address);
extern void update_pgd(unsigned long *pgd);
extern unsigned long periph_vbase;

extern unsigned long update_pte_at(unsigned long);
extern unsigned long update_l2pte_at(unsigned long);
extern unsigned long pte_update_ap(unsigned long);
extern unsigned long pte_update_domain(unsigned long);
extern int pte_in_domain(unsigned long);
extern int paddr_in_domain(struct domain *, unsigned long);

extern unsigned long pgd_translate(unsigned long);
extern void pgd_protect(unsigned long *);

extern void new_pgd_at(unsigned long *);

#define pa_to_pfn(x) (x >> PAGE_SHIFT)
#endif

#define xhyp_tlb_flags  0
#define XHYP_TLB_FLUSH  0
#define XHYP_TLB_CLEAN  1

#define HYP_FC_MVA      0x01
#define HYP_FC_ASID     0x02
#define HYP_FC_ALLD     0x04
#define HYP_FC_ALLI     0x08
#define HYP_FC_LVL1     0x10
#define HYP_FC_LVL2     0x20


#endif
