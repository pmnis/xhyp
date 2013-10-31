/*
 * serial.c
 *
 * PL01 routines
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
#include <xhyp/serial.h>
#include <xhyp/stdlib.h>
#include <xhyp/sched.h>
#include <sys/xhyp.h>
#include <autoconf.h>

#include "hyp.h"

unsigned long periph_base = PERIPH_BASE;

struct pl01     *chip = (struct pl01 *) (PERIPH_BASE + PL01_OFFSET);

/*
 * serial_init()
 * Initialize the UART
 */

void serial_init(void)
{
	unsigned long div;
	unsigned long tmp;
	unsigned long res;
	unsigned long fract;
	int f;

	f = IRQ_mask(-1);
	chip = (struct pl01 *) (periph_base + PL01_OFFSET);
	chip->ctrl = 0x0;
	tmp = 16 * 38400;
	div = PL011_CLOCK / tmp;
	res = PL011_CLOCK % tmp;
	tmp = (8 * res) / 38400;
	fract = (tmp >> 1) + (tmp & 1);

	chip->ibrd = div;
	chip->fbrd = fract;
	chip->lcrh = (UART_PL011_LCRH_WLEN_8 | UART_PL011_LCRH_FEN);
	chip->ctrl = (UART_PL011_CR_UARTEN | UART_PL011_CR_TXE | UART_PL011_CR_RXE);
	/* Clear Interrupts	*/
	chip->icr = 0xffff;
	/* Enable Interrupts	*/
#ifndef CONFIG_ARINC
	chip->imsc = PL011_IMSK;
#endif
	IRQ_mask(f);
}

int serial_echo = 1;

int serial_getc(void)
{
	int c;
	int f;

	f = IRQ_mask(-1);
	if (chip->flags & UART_PL01x_FR_RXFE) {
		IRQ_mask(f);
		return 0;
	}
	c= chip->data;
	if (serial_echo) {
		if (c == '\r')
			chip->data = '\n';
		chip->data = c;
	}
	IRQ_mask(f);
	return c;
}

int pl01x_putc (char c)
{
	int f;

	f = IRQ_mask(-1);
	if (chip->flags & UART_PL01x_FR_TXFF) {
		IRQ_mask(f);
		return 0;
	}
	chip->data = c;
	IRQ_mask(f);
	return 1;
}

int serial_putc (const char c)
{
        if (c == '\n')
                if (!pl01x_putc ('\r'))
			return 0;
        return pl01x_putc (c);
}

int serial_puts (const char *s)
{
	int n = 0;

        while (*s)
                if (serial_putc (*s++))
			n++;
		else
			break;
	return n;
}

int serial_write(const char *s, int cnt)
{
	register int n = 0;
        while (n < cnt)
                if (serial_putc (*s++))
			n++;
		else
			break;
	return n;
}

char *gets(char *str)
{
	char *s = str;
	char c;

	do {
		c = (char) serial_getc();
		if (c == 0) {
			_hyp_idle();
			continue;
		}
		if (c == '\r' || c == '\n') {
			*s = 0;
			return str;
		}
		*s++ = c;
	} while(1);
}

int puts(char *s)
{
	int n = 0;

	while(*s) {
		if (!serial_putc(*s))
			_hyp_idle();
		s++;
		n++;
	}
	return n;
}


/*
void printk(const char *fmt, ...)
{
        va_list args;
        int size = 128;
        char buf[size];

        va_start(args, fmt);
        vsnprintf(buf, size, fmt, args);
        va_end(args);
	puts(buf);
}
*/

