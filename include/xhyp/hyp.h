/*
 * hyp.h
 *
 * General hypervisor definitions
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


#define _HYP_syscall		0
#define _HYP_console		1
#define _HYP_yield		2
#define _HYP_getpage		3
#define _HYP_freepage		4
#define _HYP_irq_request	5
#define _HYP_irq_enable		6
#define _HYP_irq_disable	7
#define _HYP_irq_return		8
#define _HYP_exit		9
#define _HYP_cpuctrl		10
#define _HYP_syscall_request	11
#define _HYP_syscall_return	12
#define _HYP_pgfault_request	13
#define _HYP_undef_request	14
#define _HYP_enable_mmu		15
#define _HYP_setmode		16
#define _HYP_set_domain		17
#define _HYP_disable_cache      18
#define _HYP_reset              19
#define _HYP_idle               20
#define _HYP_flush_cache                21
#define _HYP_flush_icache               22
#define _HYP_flush_user_cache_range     23
#define _HYP_coherent_user_rang         24
#define _HYP_flush_kern_dcache_area     25
#define _HYP_dma_inv_range              26
#define _HYP_dma_clean_range            27
#define _HYP_dma_flush_range            28
#define _HYP_dchache_clean_area         29
#define _HYP_switch_mm			30
#define _HYP_set_pte			31
#define _HYP_tlb_flush			32
#define _HYP_enable_cache		33
#define _HYP_probe			34
#define _HYP_switch_task		35
#define _HYP_copy_page			36
#define _HYP_clear_page			37
#define _HYP_new_pgd			38
#define _HYP_set_pmd			39
#define _HYP_set_pte_ext		40
#define _HYP_get_pmdval			41
#define _HYP_get_timer			42
#define _HYP_io_open			43
#define _HYP_io_read			44
#define _HYP_io_write			45
#define _HYP_io_ioctl			46
#define _HYP_io_close			47
#define _HYP_event_send			48
#define _HYP_preempt_disable		49
#define _HYP_preempt_enable		50
#define _HYP_hyp			51
#define _HYP_abt_return			52
#define _HYP_usr_return			53
#define _HYP_get_tls			54
#define _HYP_cmpxchg			55
#define _HYP_cmpxchg64			56

#define _HYP_CALLS			57

#ifndef __ASSEMBLY__

typedef int (*call_entry_t)(void);
typedef unsigned long (*v_handler_t)(unsigned long);

extern int hyp_syscall(void);
extern int hyp_console(void);
extern int hyp_yield(void);
extern int hyp_getpage(void);
extern int hyp_freepage(void);
extern int hyp_irq_request(void);
extern int hyp_irq_enable(void);
extern int hyp_irq_disable(void);
extern int hyp_irq_return(void);
extern int hyp_exit(void);
extern int hyp_cpuctrl(void);
extern int hyp_syscall_request(void);
extern int hyp_syscall_return(void);
extern int hyp_pgfault_request(void);
extern int hyp_undef_request(void);
extern int hyp_enable_mmu(void);
extern int hyp_setmode(void);
extern int hyp_set_domain(void);
extern int hyp_disable_cache(void);
extern int hyp_reset(void);
extern int hyp_idle(void);
extern int hyp_flush_cache(void);
extern int hyp_flush_icache(void);
extern int hyp_flush_user_cache_range(void);
extern int hyp_coherent_user_rang(void);
extern int hyp_flush_kern_dcache_area(void);
extern int hyp_dma_inv_range(void);
extern int hyp_dma_clean_range(void);
extern int hyp_dma_flush_range(void);
extern int hyp_dchache_clean_area(void);
extern int hyp_switch_mm(void);
extern int hyp_set_pte(void);
extern int hyp_tlb(void);
extern int hyp_enable_cache(void);
extern int hyp_probe(void);
extern int hyp_task_switch(void);
extern int hyp_copy_page(void);
extern int hyp_clear_page(void);
extern int hyp_new_pgd(void);
extern int hyp_set_pmd(void);
extern int hyp_set_pte_ext(void);
extern int hyp_get_pmdval(void);
extern int hyp_get_timer(void);
extern int hyp_io_open(void);
extern int hyp_io_read(void);
extern int hyp_io_write(void);
extern int hyp_io_ioctl(void);
extern int hyp_io_close(void);
extern int hyp_event_send(void);
extern int hyp_preempt_disable(void);
extern int hyp_preempt_enable(void);
extern int hyp_hyp(void);
extern int hyp_usr_return(void);
extern int hyp_get_tls(void);
extern int hyp_cmpxchg(void);
extern int hyp_cmpxchg64(void);

#define HYPCMD_DOM_GET	0
#define HYPCMD_DOM_STOP	1
#define HYPCMD_DOM_RESTART	2
#define HYPCMD_DMESG	3
#define HYPCMD_EVENTS	4
#define HYPCMD_GET_PLAN	5
#define HYPCMD_SET_PLAN	6

extern call_entry_t    hypercall_table[_HYP_CALLS];
extern int hypercall_count[_HYP_CALLS];

extern void panic(struct context *, char *);
extern void hyp_mode_set(unsigned long mode);

#endif
