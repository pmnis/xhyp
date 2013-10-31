/*
 * fifo.h
 *
 * fifo definitions
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
#ifndef __FIFO_H
#define __FIFO_H

struct fifo {
	unsigned int ri;
	unsigned int wi;
	unsigned int sz;
	void *rp;
	void *wp;
};

extern unsigned int fifo_get(struct fifo *f, void *p, unsigned int cnt);
extern unsigned int fifo_put(struct fifo *f, void *p, unsigned int cnt);

static inline int fifo_empty(struct fifo *f)
{
	return (f->ri == f->wi);
}
static inline void fifo_init(struct fifo *f, int sz)
{
	f->wi  = 0;
	f->ri  = 0;
	f->sz  = sz;
}
static inline int fifo_full(struct fifo *f)
{
	return (f->sz - (f->ri - f->wi) > 1) ;
}

#endif
