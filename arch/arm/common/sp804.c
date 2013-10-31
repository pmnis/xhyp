/*
 * timer.c
 *
 * TIMER routines
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
#include <xhyp/timer.h>
#include <xhyp/stdlib.h>
#include <xhyp/irq.h>
#include <xhyp/sched.h>
#include <xhyp/event.h>
#include <xhyp/sp804.h>

struct sp804     *timer = (struct sp804 *) (PERIPH_BASE + SP804_OFFSET);
unsigned long	timer_freq = TIMER_PERIOD;

void timer_init(void)
{
	timer = (struct sp804 *) (periph_base + SP804_OFFSET);
}

unsigned long timer_set(unsigned long val, unsigned long ctrl)
{
	unsigned long v;

	v = timer->ctrl;
	v &= ~SP804_ENABLE;	/* Disable timer	*/
	timer->ctrl = v;
	timer->load = val;
	v |= ctrl | SP804_ENABLE;	/* Enable timer		*/
	timer->ctrl = v;
	return v;
}

unsigned long jiffies = 0;
unsigned long period = 0;
unsigned long tval = 0;
int in_timer = 1;

int timer_handler(int irq, unsigned long data)
{
	in_timer = 1;
	jiffies++;
	tval = timer->val;
	event_new(EVT_IRQ);
	//debinfo("%ld %lx\n", jiffies, tval);
	timer->ctrl &= ~ SP804_ENABLE;
	timer->itclear = 0;
	//timer->load = period;
	timer->ctrl |= SP804_ENABLE;
	sched->slice();		/* This is a scheduler entry	*/
	in_timer = 0;
	return 0;
}

/* period is set as ms		*/
unsigned long timer_periodic_ms(unsigned long ms)
{
	unsigned long v;
	unsigned long f;

	f = SP804_PERIODIC|SP804_32BITS|SP804_INTENABLE;
	/* v = 0x000186A0; Value given by ARM for a 100MHZ clock	*/
	/* It seems we run at 1MHZ on qemu...				*/
	/* 1ms -> v = 1000 = 0x3E8					*/
	v = 0x00003E8;
	v = ms * v ;
	period = v;
	v = timer_set(v , f);
	v = irq_request(IRQ_SHIFT_TIMER, timer_handler);
	return v;
}

/* 
 * RETURN VALUE en micro seconds
 */
unsigned long timer_get(void)
{
	return (period - timer->val);
}

