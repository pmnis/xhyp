/*
 * abort.c
 *
 * Abort and prefetch handling
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

#include <xhyp/config.h>
#include <xhyp/abort.h>
#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/domains.h>
#include <xhyp/hyp.h>
#include <xhyp/debug.h>
#include <xhyp/shared_page.h>


/*
 * analyse_fault
 * return 0 if abort can be handled by the domain
 */
int analyse_fault(unsigned long fsr, unsigned long far)
{
	unsigned long fault;
	unsigned long domain;

	fault = fsr & 0x0f;
	domain = (fsr & 0xf0) >> 4;
	debabt("fsr = 0x%08x\n", fsr);

	switch (fault) {
		case 0x01:
		case 0x03:
		debabt("Alignment fault\n");
			return 1;
		case 0x0c:
		debabt("External abort on first level translation\n");
			return 1;
		case 0x0e:
		debabt("External abort on second level translation\n");
		debabt("Faulting domain %d\n", domain);
			break;
		case 0x05:
		debabt("Page fault 0x05\n");
			break;
		case 0x07:
		debabt("Page fault 0x07\n");
		debabt("Faulting domain %d\n", domain);
			break;
		case 0x09:
		debabt("Page domain fault 0x09\n");
		debabt("Faulting domain %d\n", domain);
			break;
		case 0x0b:
		debabt("Page domain fault 0x0b\n");
		debabt("Faulting domain %d\n", domain);
			break;
		case 0x0d:
		debabt("Page permission fault 0x0d\n");
		debabt("Faulting domain %d\n", domain);
			break;
		case 0x0f:
		debabt("Page permission fault 0x0f\n");
		debabt("Faulting domain %d\n", domain);
			break;
		case 0x08:
		case 0x0a:
		debabt("External abort\n");
			return 1;
		default:
		debabt("Unknown error 0x%02x\n", fault);
			return 1;
	}
	debabt("far  = 0x%08x\n", far);
	return 0;
}
/*
 */
int abt_count = 95;

void do_abort(unsigned long far, unsigned long dfsr)
{
	unsigned long *p;
	struct shared_page *s = current->sp;

	debabt("dfsr: %08lx\n", dfsr);
	debabt("far : %08lx\n", far);
	debabt("pc  : %08lx\n", _context->sregs.pc);

	if (analyse_fault(dfsr, far)) {
		panic(NULL, "Unimplemented");
	}


if (abt_count-- <= 0) {
		debabt("STOP\n");
		while(1);
	}
	/* if no abort handler kill the domain		*/
	if (! s->context_abt.sregs.pc) {
		debpanic("Data abort but no abort handler\n");
		debpanic("sp....: %08lx\n", _context->sregs.sp);
		debpanic("lr....: %08lx\n", _context->sregs.lr);
		debpanic("pc....: %08lx\n", _context->sregs.pc);
		debpanic("spsr..: %08lx\n", _context->sregs.spsr);
		debpanic("tbl_l1: %08lx\n", current->tbl_l1);
		debpanic("far...: %08lx\n", far);
		p = (unsigned long *)current->tbl_l1;
		debpanic("desc..: %08lx\n", p[(far >> 20)]);
		debpanic("rights: %08lx\n", current->rights);
		sched->kill(current);
	}

	/* Save old mode and set new mode to ABT	*/
	mode_new(current, DMODE_ABT);

	/* add fault address and cause		*/
	current->ctx.regs.regs[0] = far;
	current->ctx.regs.regs[1] = dfsr;

	debabt("\n");
}

