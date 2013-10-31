/*
 * panic.c
 *
 * panic routines
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

#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/domains.h>
#include <xhyp/debug.h>
#include <xhyp/event.h>

//unsigned long debug_level = DEB_F1;
//unsigned long debug_level = DEB_ALL;
//unsigned long debug_level = DEB_PANIC|DEB_IRQ|DEB_INFO|DEB_SCHED;
//unsigned long debug_level = DEB_PANIC|DEB_IRQ|DEB_INFO|DEB_CTX;
//unsigned long debug_level = DEB_PANIC|DEB_IRQ|DEB_INFO;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_CACHE|DEB_ABT|DEB_PTE;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_HYP|DEB_IRQ|DEB_SCHED|DEB_CTX;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_HYP|DEB_PTE|DEB_CTX;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_HYP|DEB_PTE;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_IRQ;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_SCHED|DEB_CTX;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_ABT|DEB_IRQ|DEB_SCHED;
unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_ABT;
//unsigned long debug_level = DEB_PANIC|DEB_INFO|DEB_SCHED;
//unsigned long debug_level = DEB_PANIC|DEB_INFO;
//unsigned long debug_level = DEB_PANIC;
//

int PANIC = 0;

void panic(struct context *ctx, char *s)
{
	unsigned long r;

	if (PANIC++) while(1);	/* Avoid double fault */
	r = _cpu_it_disable(0);
	printk("PANIC: %s\n",s);
	debug_level |= DEB_CTX | DEB_PANIC ;
	if (ctx) {
		deb_printf(DEB_PANIC, "Registers dump...at :\n", ctx);
		show_ctx(ctx);
	} else {
		if (current) {
			deb_printf(DEB_PANIC, "Context dump...:\n", &current->ctx);
			show_ctx(_context);
		}
	}
	printk("CPSR    : %08lx\n", _get_cpsr());
	printk("CTRL[0] : %08lx\n", _get_c0());
	printk("CTRL[1] : %08lx\n", _get_c1());
	printk("CTRL[2] : %08lx\n", _get_c2());
	printk("CTRL[3] : %08lx\n", _get_c3());
	printk("DFSR    : %08lx\n", _get_dfsr());
	printk("IFSR    : %08lx\n", _get_ifsr());
	printk("FAR     : %08lx\n", _get_far());
	printk("PANIC: %s\n",s);
	printk("MSR: 0x%08x\n",r);
//	event_dump();

	dump_pgd((unsigned long *)current->tbl_l1);
	while(1)
		;
}

