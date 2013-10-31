/*
 * ring.h
 *
 * ring definitions
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
#ifndef __RING_H
#define __RING_H

struct ring {
	unsigned int ri;
	unsigned int wi;
	unsigned int sz;
	void *p;
	unsigned int flags;
};

extern unsigned int ring_get(struct ring *f, void *p, unsigned int cnt);
extern unsigned int ring_put(struct ring *f, void *p, unsigned int cnt);
extern struct ring trace_ring;

static inline int ring_empty(struct ring *f)
{
	return (f->ri  == f->wi);
}
#define RING_FULL	1
static inline void ring_init(struct ring *f, void *p, int sz)
{
	f->wi  = 0;
	f->ri  = 0;
	f->sz  = sz;
	f->p  = p;
	f->flags = 0;
}

#endif
