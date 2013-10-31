/*
 * pl190.c
 *
 * Interrupt handling
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
#include <xhyp/irq.h>
#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/pl190.h>
#include <xhyp/debug.h>


struct pl190	*pic = (struct pl190 *) (PERIPH_BASE + PIC_OFFSET);;

void pic_init(void)
{
	debirq("\n");
	pic = (struct pl190 *) (periph_base + PIC_OFFSET);
	pic->enable = 0;
}

unsigned long pic_clear(int irq)
{
	unsigned long mask;

	mask = 0x01 << irq;
	pic->enable_clear = mask;

	return mask;
}

unsigned long pic_enable(int irq)
{
	unsigned long mask;

	mask = 0x01 << irq;
	pic->enable |= mask;

	return mask;
}

unsigned long pic_status(void)
{
	return pic->irq_status;
}

