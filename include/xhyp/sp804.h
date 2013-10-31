/*
 * sp804.h
 *
 * timer definitions
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
#ifndef _SP804_H
#define _SP804_H

struct sp804 {
	unsigned long load;
	unsigned long val;
	unsigned long ctrl;
	unsigned long itclear;
	unsigned long itstatus;
	unsigned long itmask;
	unsigned long bgload;
};

#define SP804_ENABLE	0x80
#define SP804_PERIODIC	0x40
#define SP804_INTENABLE	0x20
#define SP804_DIV16	0x08
#define SP804_DIV256	0x04
#define SP804_32BITS	0x02
#define SP804_ONESHOT	0x01

#define SP804_OFFSET    0x001E2000

#endif
