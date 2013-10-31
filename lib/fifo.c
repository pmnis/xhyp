/*
 * fifo.c
 *
 * fifo functions
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

#include <sys/fifo.h>
#include <xhyp/stdlib.h>
#include <xhyp/debug.h>

unsigned int fifo_get(struct fifo *f, void *p, unsigned int cnt)
{
	unsigned int ro;
	unsigned int n;

	ro = f->ri % f->sz;
	cnt = (cnt < (f->wi - f->ri))? cnt : f->wi - f->ri;
	n = (cnt < (f->sz - ro))? cnt : f->sz - ro;
	memcpy(p, f->rp + ro, n);
	memcpy(p + n, f->rp, cnt - n);
	f->ri += cnt;
	return cnt;
}

unsigned int fifo_put(struct fifo *f, void *p, unsigned int cnt)
{
	unsigned int wo;
	unsigned int n;

	wo = f->wi % f->sz;
	cnt = (cnt < (f->sz - (f->wi - f->ri)))? cnt : f->sz - (f->wi - f->ri);
	n = (cnt < (f->sz - wo))? cnt : f->sz - wo;
	memcpy(f->wp + wo, p, n);
	memcpy(f->wp, p + n, cnt - n);
	f->wi += cnt;
	if (f->ri > f->sz ) {
		f->wi -= f->sz;
		f->ri -= f->sz;
	}
	return cnt;
}


