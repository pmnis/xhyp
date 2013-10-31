/*
 * mm.c
 *
 * Memory managment
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

#include <xhyp/lowlevel.h>
#include <xhyp/mmu.h>
#include <xhyp/domains.h>
#include <xhyp/mm.h>

unsigned long free = (unsigned long )&__hyp_end;

 void *mallocl_linear(int size, unsigned long align)
{
	unsigned long p = free;

	deballoc("free: %08lx size %08lx\n", p, size);
	if (p & (align - 1)) {
		p &= ~(align - 1);
		p += align;
	}
	free = p + size;

	deballoc("free: %08lx p: %08lx\n", free, p);
	return (void *) p;
}

struct page *page_table;
unsigned long	mem_size = MEMORY_SIZE;

void init_page_table(void)
{
	int size;

	size = ((mem_size >> SECTION_SHIFT) * sizeof(struct page));
	page_table = mallocl_linear(size, PAGE_SIZE);
	memset(page_table, 0, size * sizeof(unsigned long));
}

void alloc_coarse_tables(struct domain *d)
{
	int size;

	size = PMD_SIZE * NB_PMD_ENTRIES;
	d->tbl_l2 = mallocl_linear(size, PMD_ALIGN);
	memset(d->tbl_l2, 0, size);
	deballoc("domain %d: tbl_l2: %08lx\n", d->id, d->tbl_l2);
}

void alloc_page_tables(struct domain *d)
{
	int size;

	size = PGD_SIZE;
	d->tbl_l1 = (unsigned long )mallocl_linear(size, PGD_ALIGN);
	memset((void *)d->tbl_l1, 0, size);

	alloc_coarse_tables(d);
}


