/*
 * lowlevel.h
 *
 * lowlevel definitions
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

#ifndef __LOWLEVEL_H
#define __LOWLEVEL_H

extern unsigned long _get_c0_ID(void);
extern unsigned long _get_c0_CT(void);
extern unsigned long _get_c0_TCM(void);
extern unsigned long _get_c0(void);
extern unsigned long _get_c1(void);
extern unsigned long _get_c2(void);
extern unsigned long _get_c3(void);
extern unsigned long _get_cpsr(void);
extern unsigned long _get_spsr(void);
extern unsigned long _get_dfsr(void);
extern unsigned long _get_ifsr(void);
extern unsigned long _get_far(void);
extern void _set_c3(unsigned long);
extern void _setup_mmu(unsigned long);

struct cr0 {
	unsigned int unused:16;
	unsigned int L:1;
	unsigned int r:1;
	unsigned int V:1;
	unsigned int I:1;
	unsigned int sbz:2;
	unsigned int R:1;
	unsigned int S:1;
	unsigned int B:1;
	unsigned int sbo:4;
	unsigned int C:1;
	unsigned int A:1;
	unsigned int M:1;
};


extern void prepare_mmu(unsigned long, unsigned long);
extern void _hypercall(void);
extern void * _io_remap_page(void *);
extern void domain01(void);
extern void domain02(void);
extern void * __ttb;
extern void * __end_of_all;
extern unsigned long pt_level1;

extern void _switch_to(void);

extern unsigned long _setup_domain(unsigned long);

extern unsigned long _cpu_it_disable(unsigned long);
extern unsigned long _cpu_it_enable(unsigned long);
extern unsigned long _cache_flush(unsigned long);
extern unsigned long _clean_d_entry(unsigned long);
extern void _wfi(void);
extern void _wfi2(void);
extern void _load_tlb(unsigned long);
extern unsigned long _hyp_ttb;
extern unsigned long _hyp_rights;
extern unsigned long _dom_ttb;
extern unsigned long _dom_rights;

#define NULL	0
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

extern unsigned long __hyp_end;

#endif
