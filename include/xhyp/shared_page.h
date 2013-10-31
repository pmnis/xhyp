/*
 * shared_page.h
 *
 * Shared page definitions
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
#ifndef __SHARED_PAGE_H
#define __SHARED_PAGE_H

#include <xhyp/context.h>
#include <sys/ports.h>

#define NB_QUEUING_PORT	4*16
#define XHYP_SP_MAGIC	0xCAFEC0CA
#define XHYP_SP_MAGIC2	0xDEADBEEF
struct shared_page {
	unsigned long	magic;
	unsigned long	cp15_c0;
	unsigned long	cp15_c1;
	unsigned long	v_pgd;
	struct context	context_usr;
	struct context	context_und;
	struct context	context_abt;
	struct context	context_sys;
	struct context	context_irq;
	unsigned long	jiffies;
	unsigned long	irq;
	unsigned long	last_state;
	unsigned long 	v_cpsr;
	unsigned long 	v_spsr;
	unsigned long 	v_irq_enabled;
	unsigned long 	v_irq_pending;
	unsigned long 	v_irq_ack;
	unsigned long 	v_irq_mask;
	unsigned long	sampling_port;
	struct queuing_port	qp[NB_QUEUING_PORT];
	unsigned long	end_magic;
};

#endif
