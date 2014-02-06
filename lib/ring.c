/*
 * ring.c
 *
 * ring functions
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

#include <xhyp/ring.h>
#include <xhyp/stdlib.h>
#include <xhyp/debug.h>

unsigned int ring_get_entry(struct ring *f, void *p, unsigned int size, unsigned int num)
{
	unsigned int ro;
	unsigned int cnt;
	unsigned int n = f->wi - f->ri;

	cnt = size * num;
	if (cnt > n)
		return 0;
	ro = f->wi % f->sz;
	if ( cnt <= ro ) {
		memcpy(p, f->p + ro - cnt, size);
		return size;
	}
	n = cnt - ro;
	memcpy(p, f->p + f->sz - n, size);
	return size;
}

unsigned int ring_copy(struct ring *f, void *p, unsigned int cnt, int modify)
{
	unsigned int ro;
	unsigned int n = f->wi - f->ri;

	if (!n)
		return 0;
	if (cnt > n)
		return 0;
	ro = f->ri % f->sz;
	n = (cnt < (f->sz - ro))? cnt : f->sz - ro;
	memcpy(p, f->p + ro, n);
	memcpy(p + n, f->p, cnt - n);
	if (modify) f->ri += cnt;
	return cnt;
}

unsigned int ring_get(struct ring *f, void *p, unsigned int cnt)
{
	return ring_copy(f, p, cnt, 1);
}

unsigned int ring_dump(struct ring *f, void *p, unsigned int cnt)
{
	return ring_copy(f, p, cnt, 0);
}

unsigned int ring_put(struct ring *f, void *p, unsigned int cnt)
{
	unsigned int wo;
	unsigned int n;

	wo = f->wi % f->sz;
	n = (cnt < (f->sz - wo))? cnt : f->sz - wo;
	memcpy(f->p + wo, p, n);
	memcpy(f->p, p + n, cnt - n);
	f->wi += cnt;
	if (f->wi  > (2 * f->sz))
		f->flags |= RING_FULL;
	if (f->flags & RING_FULL)
		f->ri = f->wi - f->sz;
	if (f->ri > f->sz ) {
		f->wi -= f->sz;
		f->ri -= f->sz;
	}
	return cnt;
}


