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

void time_get(struct timespec *ts)
{
	unsigned long t = HZ;

	ts->tv_sec = jiffies / t;
	ts->tv_usec = (jiffies % t) * ((MICROS_PER_SEC) / t) ;
	t = ts->tv_usec + timer_get();
	if (t > (MICROS_PER_SEC)) {
		t -= MICROS_PER_SEC;
		ts->tv_sec--;
	}
	ts->tv_usec = t;
}
struct timespec xtime;
void time_update(void)
{
	time_get(&xtime);
}

void time_tick(void)
{
	int i;

	for(i = 0; i < nb_domains; i++)
		domain_table[i].jiffies++;
	sched->slice();
}

