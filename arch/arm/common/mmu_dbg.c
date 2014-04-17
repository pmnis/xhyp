/*
 * mmu_dbg.c
 *
 * Memory management debug
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
#include <xhyp/domains.h>
#include <xhyp/mm.h>
#include <xhyp/stdlib.h>
#include <xhyp/debug.h>
#include <xhyp/hyp.h>

#ifdef DEBUG
void show_pte(unsigned long *pte)
{
	debinfo("pte %08lx %08lx\n", pte, *pte);
}

void show_section(unsigned long *s)
{
	unsigned char d;
	unsigned char p;

	d = (*s >> 5  & 0x0f);
	p = (*s >> 10 & 0x03);
	printk("Section %08x d: %02lx p %02lx to %08x\n",s, d, p, *s);
}

void show_ventry(unsigned long *pgd, unsigned long vaddr)
{
	unsigned long *l1;
	unsigned long *l2;
	unsigned long idx;

	idx = (vaddr >> 20) & 0x0fff;
	debpte("pgd %08lx vaddr %08lx idx %08lx\n", pgd, vaddr, idx);
	l1 = pgd + idx;
	debpte("%08lx %08lx %08lx\n", pgd, vaddr, l1);
	debinfo("pgd %08lx vaddr %08lx *l1 %08lx\n", pgd, vaddr, *l1);
	switch (*l1 & 0x03) {
	case 0x01:
		l2 = (unsigned long *) (*l1 & 0xfffffc00);
		l2 += (vaddr >> 12) & 0xff;
		show_pte(l2);
		break;
	case 0x02:
		show_section(l1);
		break;
	case 0x00:
		debpte("Unmapped\n");
		break;
	default:
		debpte("ERROR\n");
		{
		panic(NULL, "UNKNOWN PTE");
		}
		break;
	}
}

void show_coarse(unsigned long *c)
{
	unsigned char d = (*c >> 5 & 0xff);
	unsigned long t = (*c & 0xfffffc00);
	int nb_show = 256;
	int i = 0;
	unsigned long *pte;

	debinfo("Coarse %08x d: %01x to %08x\n",c, d, t);

	while (nb_show-- > 0) {
		//show_pte((unsigned long *)(t + 4 * i));
		pte = (unsigned long *)(t + 4 * i);
		debinfo("--> [%02x] pte %08lx %08lx\n", i, pte, *pte);
		i++;
	}
}
int show_entry(unsigned long *entry)
{
	int type = *entry & 0x03;

	switch (type) {
	case 0x00:
		debpte("IGNORED...: %08lx : %08x\n",entry, *entry);
		break;
	case 0x01:
		debpte("COARSE....: %08lx : %08x\n",entry, *entry);
		show_coarse(entry);
		break;
	case 0x02:
		show_section(entry);
		break;
	case 0x03:
		debpte("SUBSECTION: %08lx : %08x\n",entry, *entry);
		break;
	}
	return type;
}
int show_level1(unsigned long *entry)
{
	switch (*entry & 0x03) {
	case 0x00:
		debpte("IGNORED at %08lx: %08x\n", entry, *entry);
		break;
	case 0x01:
		debpte("COARSE  at %08x: %08x\n", entry, *entry);
		show_coarse(entry);
		break;
	case 0x02:
		show_section(entry);
		break;
	case 0x03:
		debpte("SUBSECTION %08x: %08lx\n", entry, *entry);
		break;
	}
	return 0;
}

void show_pgd(unsigned long *pgd)
{
	int i;

	debpte("PGD AT %08x\n", pgd);
	for (i = 0; i < 0x1000; i++) {
		if (! (i % 8))
			printk("\n%03x ", i);
		printk("%08lx ", pgd[i]);
	}
	debpte("PGD END %08x\n", pgd);
}
#endif
