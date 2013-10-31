/*
 * tlb.c
 *
 * Gestion du TLB
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

#include <xhyp/mmu.h>
#include <xhyp/timer.h>
#include <xhyp/domains.h>
#include <xhyp/hyp.h>
#include <xhyp/debug.h>
#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/soc.h>

int hyp_tlb(void)
{
	unsigned long command;
	unsigned long entry;
	const unsigned long zero = 0;

	debtlb("BEGIN\n");

	command = CTX_arg0;
	entry = CTX_arg1;

	/* Invalidate entire TLB	*/
	asm("mcr	p15, 0, %0, c8, c7, 0" : : "r" (zero) : "cc");
	/* Invalidate L2 Cache	*/
	/* TBD : if SMP only */
	asm("mcr	p15, 0, %0, c7, c10, 1" : : "r" (entry) : "cc");
	asm("mcr	p15, 0, %0, c15, c9, 1" : : "r" (entry) : "cc");

	if (command == XHYP_TLB_FLUSH)
		asm("mcr	p15, 0, %0, c7, c10, 4" : : "r" (zero) : "cc");

	deb_printf(DEB_TLB, "END\n");
	return 0;
}

