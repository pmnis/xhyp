/*
 * event.c
 *
 * Systeme de trace par evenement
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

#include <xhyp/mmu.h>
#include <xhyp/timer.h>
#include <xhyp/domains.h>
#include <xhyp/shared_page.h>
#include <xhyp/hyp.h>
#include <xhyp/debug.h>
#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/soc.h>
#include <xhyp/irq.h>

#include <sys/io.h>
#include <xhyp/ring.h>
#include <xhyp/event.h>

struct ring trace_ring;
struct event event_buffer[MAX_EVENT];

evt_hook_t	evt_hook[EVT_NB];

char *event_str[] = {
	"start", "sched_in", "sched_out", "irq", "irq_in", "irq_out",
	"irq_ret", "sys_in", "sys_out", "abt", "abt_in", "abt_out",
	"abt_ret", "wfi"
};

void evt_hook_schedin(struct event *e)
{
	current->t_mode = current->mode;

	switch (current->t_mode) {
	case DMODE_IRQ:
		current->t_irqin = e->timestamp;
		break;
	case DMODE_ABT:
		current->t_abtin = e->timestamp;
		break;
	case DMODE_SVC:
		current->t_sysin = e->timestamp;
		break;
	case DMODE_USR:
		current->t_usrin = e->timestamp;
		break;
	}
}

void adjust_time(struct timespec *dst, struct timespec *t0, struct timespec *t1)
{
	unsigned long sec, usec;

	sec = t1->tv_sec - t0->tv_sec;
	usec = t1->tv_usec - t0->tv_usec;
	if (t1->tv_usec < t0->tv_usec) {
		sec--;
		usec += MICROS_PER_SEC;
	}

	if (sec < 0) debinfo("timer error\n");
	if (sec == 0 && usec < 0) debinfo("timer error\n");

	sec += dst->tv_sec;
	usec += dst->tv_usec;
	if (usec > MICROS_PER_SEC) {
		usec -= MICROS_PER_SEC;
		sec += 1;
	}
	dst->tv_sec = sec;
	dst->tv_usec = usec;
	
}
void evt_hook_schedout(struct event *e)
{

	switch (current->t_mode) {
	case DMODE_IRQ:
		adjust_time(&current->t_irq, &current->t_irqin, &e->timestamp);
		break;
	case DMODE_ABT:
		adjust_time(&current->t_abt, &current->t_abtin, &e->timestamp);
		break;
	case DMODE_SVC:
		adjust_time(&current->t_sys, &current->t_sysin, &e->timestamp);
		break;
	case DMODE_USR:
		adjust_time(&current->t_usr, &current->t_usrin, &e->timestamp);
		break;
	}
}

void evt_hook_abtin(struct event *e)
{
	current->t_abtin = e->timestamp;
}

void evt_hook_abtout(struct event *e)
{
	adjust_time(&current->t_abt, &current->t_abtin, &e->timestamp);
}

void evt_hook_irqin(struct event *e)
{
	current->t_irqin = e->timestamp;
}

void evt_hook_irqout(struct event *e)
{
	adjust_time(&current->t_irq, &current->t_irqin, &e->timestamp);
}
void evt_hook_irqret(struct event *e)
{

	current->t_mode = current->mode;
	switch (current->t_mode) {
	case DMODE_IRQ:
		current->t_irqin = e->timestamp;
		break;
	case DMODE_ABT:
		current->t_abtin = e->timestamp;
		break;
	case DMODE_SVC:
		current->t_sysin = e->timestamp;
		break;
	case DMODE_USR:
		current->t_usrin = e->timestamp;
		break;
	}

}
void evt_hook_wfi(struct event *e)
{
	adjust_time(&current->t_irq, &current->t_irqin, &e->timestamp);
}

void event_init(void)
{
	ring_init(&trace_ring, event_buffer, MAX_EVENT * sizeof(struct event));
	evt_hook[EVT_IRQIN] = evt_hook_irqin;
	evt_hook[EVT_IRQOUT] = evt_hook_irqout;
	evt_hook[EVT_IRQRET] = evt_hook_irqret;
	evt_hook[EVT_SCHEDIN] = evt_hook_schedin;
	evt_hook[EVT_SCHEDOUT] = evt_hook_schedout;
	evt_hook[EVT_ABTIN] = evt_hook_abtin;
	evt_hook[EVT_ABTOUT] = evt_hook_abtout;
	evt_hook[EVT_WFI] = evt_hook_wfi;
}

int event_nr = 0;

int event_new(int e)
{
	struct event event = {};

	time_get(&event.timestamp);
	event.event = e;
	event.nr = event_nr++;
	event.syscall = current->syscall;
	event.c_mode = current->mode;
	event.o_mode = current->old_mode;
	event.id = current->id;
	event.state = current->state;

	ring_put(&trace_ring, &event, sizeof(event));

	if (evt_hook[e])
		evt_hook[e](&event);

	return 0;
}

char *mode_str[DMODE_SIZE] = { "DEAD", "SVC", "IRQ", "USR", "ABT", "UND", "FIQ" };

void event_show(struct event *event)
{
	printk("%12ld %5d %2d %2d %12s %2d %08s %08s\n",
		event->timestamp, event->nr,
		event->id, event->event,
		event_str[event->event], event->syscall,
		mode_str[event->c_mode], mode_str[event->o_mode]);
}

void event_dump(void)
{
	struct event event;

	while (ring_get(&trace_ring, &event, sizeof(event)))
		event_show(&event);
}

