/*
 * abort.h
 *
 * abort definitions
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

#ifndef __ABORT_H
#define __ABORT_H

void do_abort(unsigned long, unsigned long);
int hyp_abt_return(void);

#define XHYP_FLT_TRANSLAT	0x01
#define XHYP_FLT_PAGE		0x02
#define XHYP_FLT_SEC		0x04
#define XHYP_FLT_WRITE		0x08
#define XHYP_FLT_PREFETCH	0x10

#endif
