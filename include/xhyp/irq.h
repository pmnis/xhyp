/*
 * irq.h
 *
 * irq definitions
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

#ifndef __IRQ_H
#define __IRQ_H

/* These function are defined by appropriate PIC handler */
extern void pic_init(void);
extern unsigned long pic_disable(int irq);
extern unsigned long pic_enable(int irq);
extern unsigned long pic_clear(int irq);
extern unsigned long pic_status(void);


/* General IRQ definitions */
struct irq_action {
	int (*handler)(int, unsigned long);
	unsigned long mask;
	int	irq;
};


extern void irq_init(void);
extern int irq_request(int irq, int (*handler)(int, unsigned long));
extern void do_irq(void);

extern int need_resched;
extern void wfi(void);

/* This is the size of the irq_table */
#define NB_IRQ  32

/* This should go elsewhere, depending on hardware */
#define IRQ_SHIFT_TIMER		4
#define IRQ_SHIFT_UART		12
#define IRQ_SHIFT_EVENT		16
#define IRQ_SHIFT_QPORT		17

#define IRQ_MASK_TIMER	(1 << IRQ_SHIFT_TIMER)
#define IRQ_MASK_UART	(1 << IRQ_SHIFT_UART)
#define IRQ_MASK_EVENT	(1 << IRQ_SHIFT_EVENT)
#define IRQ_MASK_QPORT	(1 << IRQ_SHIFT_QPORT)

#endif
