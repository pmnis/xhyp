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
#include <xhyp/irq.h>
#include <xhyp/sched.h>

struct pl01     *chip = (struct pl01 *) (PERIPH_BASE + PL01_OFFSET);

#define MAX (1 << 20)
void msleep(int n)
{
        int i,j;
        for (i = 0; i < n; i++)
                for (j = 0; j < MAX; j++);
}

int uart_handler(int irq, unsigned long data)
{
	unsigned int status;

	status = chip->rirq;
	if (status & PL011_TXIM)
		deb_printf(DEB_INFO, "Got TX: %08lx\n", status);
	if (status & PL011_RXIM)
		deb_printf(DEB_INFO, "Got RX: %08lx\n", status);
	chip->icr = 0xffff;
	chip->imsc = PL011_IMSK;
	return 0;
}

void serial_init(void)
{
	unsigned long div;
	unsigned long tmp;
	unsigned long res;
	unsigned long fract;

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
	if (sched->type & SCHED_IRQ)
		serial_init_irq();
}

void serial_init_irq(void)
{
	deb_printf(DEB_INFO, "INIT INTERRUPT\n");

	/* Clear Interrupts     */
	chip->icr = 0xffff;
	chip->imsc = 0;
	/* Request IRQ		*/
	irq_request(IRQ_SHIFT_UART, uart_handler);
	/* Enable Interrupts    */
	chip->imsc = PL011_IMSK;
}

int pl01x_putc (char c)
{
	while (chip->flags & UART_PL01x_FR_TXFF)
		msleep(1);
	chip->data = c;
	return 1;
}

int serial_putc (const char c)
{
        if (c == '\n')
                pl01x_putc ('\r');

        return pl01x_putc (c);
}

int serial_puts (const char *s)
{
	int n = 0;
        while (*s) {
                serial_putc (*s++);
		n++;
	}
	return n;
}

int serial_write(const char *s, int n)
{
	register int i = n;
        while (i--)
                serial_putc (*s++);
	return n;
}

