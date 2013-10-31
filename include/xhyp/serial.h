/*
 * serial.h
 *
 * PL01 definitions
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

#ifndef __SERIAL_H
#define __SERIAL_H

#include <autoconf.h>

#ifdef CONFIG_BOARD_VERSATILE
#define PL01_OFFSET     0x001F1000
#else
#ifdef CONFIG_BOARD_REALVIEW
#define PL01_OFFSET     0x00009000
#else
#error "No board configured"
#endif
#endif

struct pl01 {
	unsigned long data;		// 0
	unsigned long status;		// 4
	unsigned long reserved0;	// 8
	unsigned long reserved1;	// C
	unsigned long reserved2;	// 10
	unsigned long reserved3;	// 14
	unsigned long flags;		// 18
	unsigned long reserved4;	// 1C
	unsigned long irda;		// 20
	unsigned long ibrd;		// 24
	unsigned long fbrd;		// 28
	unsigned long lcrh;		// 2C
	unsigned long ctrl;		// 30
	unsigned long ifls;		// 34	Interrupt Fifo Select
	unsigned long imsc;		// 38	Interrupt mask
	unsigned long rirq;		// 3C	Raw IT status
	unsigned long mirq;		// 40	Masked IT status
	unsigned long icr;		// 44	Interrupt clear
	unsigned long dma;		// 48	DMA Ctrl
};

extern struct pl01     * chip;

#define PL011_CLOCK			24000000
#define UART_PL011_LCRH_WLEN_8		(3 << 5)
#define UART_PL011_LCRH_FEN		(1 << 4)
#define UART_PL011_CR_UARTEN		(1 << 0)
#define UART_PL011_CR_TXE		(1 << 8)
#define UART_PL011_CR_RXE		(1 << 9)

#define UART_PL01x_FR_TXFE		(1 << 7)
#define UART_PL01x_FR_RXFF		(1 << 6)
#define UART_PL01x_FR_TXFF		(1 << 5)
#define UART_PL01x_FR_RXFE		(1 << 4)

#define PL011_TXIM	(1 << 5)
#define PL011_RXIM	(1 << 4)

//#define PL011_IMSK	PL011_TXIM|PL011_RXIM
#define PL011_IMSK	PL011_RXIM

extern void serial_init(void);
extern int serial_puts(const char *);
extern int serial_putc(char);
extern int serial_getc(void);
extern int serial_write(const char *s, int n);
extern unsigned long serial_status(void);
extern void serial_init_irq(void);

#define write(a,b,c)	serial_write(b,c)
#endif
