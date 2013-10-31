/*
 * pl190.h
 *
 * PL190 definitions
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

#ifndef __PL190_H
#define __PL190_H

#define PIC_OFFSET      0x00140000

#define PICIRQStatus	0x0000
#define PICFIQStatus	0x0004
#define PICRawIntr	0x0008
#define PICIntSelect	0x000C
#define PICIntEnable	0x0010
#define PICIntEnClear	0x0014
#define PICSoftInt	0x0018
#define PICSoftIntClear	0x001C
#define PICProtection	0x0020
#define PICVectAddr	0x0030
#define PICDefVectAddr	0x0034
#define PICVectAddr0	0x0100
#define PICVectCtrl0	0x0200
#define PICTests	0x0300
#define PICID		0x0FE0

struct pl190 {
	unsigned long	irq_status;
	unsigned long	fiq_status;
	unsigned long	raw_intr;
	unsigned long	select;
	unsigned long	enable;
	unsigned long	enable_clear;
	unsigned long	softint;
	unsigned long	softint_clear;
	unsigned long	protection;
	unsigned long	unused00;
	unsigned long	unused01;
	unsigned long	unused02;
	unsigned long	vector;
	unsigned long	def_vector;
};

#endif
