/*
 * tag.c
 *
 * Gestion des tags
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
#include <xhyp/mmu.h>
#include <xhyp/domains.h>
#include <xhyp/lowlevel.h>
#include <xhyp/hyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/list.h>
#include <xhyp/mm.h>
#include <xhyp/debug.h>
#include <xhyp/soc.h>
#include <xhyp/shared_page.h>
#include <xhyp/setup.h>

#define CMDLINE	"console=hvc root=/dev/ram0"

unsigned long addtag_cmdline(struct tag *p)
{
	p->hdr.tag = ATAG_CMDLINE;
	p->hdr.size = sizeof(struct tag_header) + 80;
	snprintf((char *)&p->u.cmdline.cmdline[0], 80, CMDLINE);
	deb_printf(DEB_TAGS, "type: %08lx size: %08lx\n", p->hdr.tag, p->hdr.size);
	return p->hdr.size;
}

unsigned long addtag_mem(struct domain *d, struct tag *p)
{
	p->hdr.tag = ATAG_MEM;
	p->hdr.size = sizeof(struct tag_header) + sizeof(struct tag_mem32);
	p->u.mem.start = d->base_addr;
	p->u.mem.size  = d->size;
	deb_printf(DEB_TAGS, "type: %08lx size: %08lx\n", p->hdr.tag, p->hdr.size);
	return p->hdr.size;
}

unsigned long addtag_core(struct domain *d, struct tag *p)
{
	p->hdr.tag = ATAG_CORE;
	p->hdr.size = sizeof(struct tag_header) + sizeof(struct tag_core);
	p->u.core.flags = 0x01;
	p->u.core.pagesize = 0x01000;
	p->u.core.rootdev = 0;
	deb_printf(DEB_TAGS, "type: %08lx size: %08lx\n", p->hdr.tag, p->hdr.size);
	return p->hdr.size;
}

void addtag_null(struct tag *p)
{
	p->hdr.tag = 0;
	p->hdr.size = 0;
	deb_printf(DEB_TAGS, "type: %08lx size: %08lx\n", p->hdr.tag, p->hdr.size);
}

void verify_tag(unsigned long base)
{
	char *t;

	for(t = (char *) base; ((struct tag *)t)->hdr.size; t += ((struct tag *)t)->hdr.size) {
		deb_printf(DEB_TAGS, "type: %08lx size: %08lx\n", ((struct tag *)t)->hdr.tag, ((struct tag *)t)->hdr.size);
	}
}

unsigned long init_tags(struct domain *d)
{
	unsigned long tag;
	unsigned long base;

	base = d->tags_addr;
	deb_printf(DEB_TAGS, "base %08lx\n", base);
	tag = base;
	tag += addtag_core(d, (struct tag *)tag);
	deb_printf(DEB_TAGS, "tag %08lx\n", tag);
	tag += addtag_mem(d, (struct tag *)tag);
	deb_printf(DEB_TAGS, "tag %08lx\n", tag);
	tag += addtag_cmdline((struct tag *)tag);
	deb_printf(DEB_TAGS, "tag %08lx\n", tag);
	addtag_null((struct tag *)tag);
	deb_printf(DEB_TAGS, "tag %08lx\n", tag);

	verify_tag(base);

	deb_printf(DEB_TAGS, "base %08lx\n", base);
	return base;
}


