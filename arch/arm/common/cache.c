/*
 * cache.c
 *
 * Gestion des caches
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
#include <xhyp/errno.h>
#include <xhyp/shared_page.h>

int hyp_enable_cache(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_disable_cache(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_reset(void)
{
	debcache("BEGIN\n");
	return 0;
}

void dump_pgd(unsigned long *p)
{
	int i,j;

	printk("table at %08lx\n",p);
	for (j = 0; j < 0x200; j++) {
		printk("\n%08lx ",i * j);
		for (i = 0; i < 8; i++, p++)
			printk("%08lx ", *p);
	}
}

int hyp_flush_cache(void)
{
	unsigned long flags;
	unsigned long addr;
	unsigned long len;

	flags = CTX_arg0;
	addr = CTX_arg1;
	len = CTX_arg2;
	deb_printf(DEB_CACHE, "addr %08lx len %08lx\n", addr, len);

	flags &= HYP_FC_ALLI;
	_cache_flush(flags);
	return 0;
}

int hyp_flush_icache(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_flush_user_cache_range(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_coherent_user_rang(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_flush_kern_dcache_area(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_dma_inv_range(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_dma_clean_range(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_dma_flush_range(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

int hyp_dchache_clean_area(void)
{
	deb_printf(DEB_CACHE, "BEGIN\n");
	return 0;
}

