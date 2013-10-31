/*
 * mm.h
 *
 * Memory Managment definitions
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

extern void *mallocl_linear(int size, unsigned long align);
extern void alloc_page_tables(struct domain *domain);
extern unsigned long mem_size;
struct page {
	unsigned short flags;
	unsigned char domain;
	unsigned char type;
};
extern struct page *page_table;
extern void init_page_table(void);

#define PTABLE_TYPE_NONE	0x00
#define PTABLE_TYPE_HYP		0x01
#define PTABLE_TYPE_PGD		0x02
#define PTABLE_TYPE_USR		0x03

static inline struct page * hyp_to_pfn(unsigned long address)	
{
	return &page_table[(address & 0xfff00000) >> 20];
}

extern unsigned long free;
