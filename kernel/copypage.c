/*
 * copypage.c
 *
 * Gestion de la copie de pages
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

int hyp_copy_page(void)
{
	unsigned long to;
	unsigned long from;
	unsigned long size;
	unsigned long src, dst;

	deb_printf(DEB_INFO, "BEGIN\n");

	to = CTX_arg0; 
	from = CTX_arg1; 
	size = CTX_arg2; 

	deb_printf(DEB_INFO, "to %08lx from: %08lx size %08lx\n", to, from, size);
	src = virt_to_phys(current, from);
	dst = virt_to_phys(current, to);
	deb_printf(DEB_INFO, "dst %08lx src %08lx size %08lx\n", src, dst, size);
	memcpy((char *)dst, (char *)src, size);
	return 0;
}

int hyp_clear_page(void)
{
	unsigned long to;
	unsigned long size;
	unsigned long addr;

	deb_printf(DEB_INFO, "BEGIN\n");

	to = CTX_arg0; 
	size = CTX_arg1; 

	deb_printf(DEB_INFO, "to %08lx size %08lx\n", to, size);
	addr = virt_to_phys(current, to);
	deb_printf(DEB_INFO, "addr %08lx size %08lx\n", addr, size);
	memset((char *)addr, 0, size);
	return 0;
}

