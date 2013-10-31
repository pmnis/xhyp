/*
 * hyp.h
 *
 * hypervisor definitions for the domains
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
#ifndef __XHYP_H
#define __XHYP_H

#include <xhyp/soc.h>

extern void _hyp_console(volatile char *, int);
extern void _hyp_exit(int);
extern void _hyp_syscall(int);
extern void _hyp_syscall_request(void(*)(unsigned long), unsigned long *);
extern void _hyp_pgfault_request(void(*)(void *, unsigned long), unsigned long *);
extern void _hyp_undef_request(void(*)(void *), unsigned long *);
extern void _hyp_syscall_return(unsigned long);
extern void _hyp_irq_request(void(*)(unsigned long), unsigned long *);
extern void _hyp_irq_enable(unsigned long);
extern void _hyp_irq_return(unsigned long);
extern void _hyp_new_pgd(unsigned long);

extern struct shared_page *xhyp_sp;

static inline int IRQ_mask(int mask)
{
	int tmp;

	tmp = xhyp_sp->v_irq_mask;
	xhyp_sp->v_irq_mask = mask;

	return tmp;
}

static inline void IRQ_enable(void)
{
	xhyp_sp->v_cpsr &= ~dis_irqs;
}

static inline void IRQ_disable(void)
{
	xhyp_sp->v_cpsr |= dis_irqs;
}


#endif
