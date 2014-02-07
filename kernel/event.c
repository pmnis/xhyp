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

/** @file event.c
 * @brief event management
 */

/** @var struct ring trace_ring
 * @brief hypervisor ring management structure for events
 */
struct ring trace_ring;

/** @var struct event event_buffer[MAX_EVENT]
 * @brief hypervisor ring buffer holding events
 */
struct event event_buffer[MAX_EVENT];

/** @var evt_hook_t      evt_hook[EVT_NB]
 * @brief hook entries to handle events
 */
evt_hook_t	evt_hook[EVT_NB];

/** @var char *event_str[]
 * @brief readable event names
 */
char *event_str[EVT_NB] = {
	"start", "sched_in", "sched_out", "irq", "irq_in", "irq_out",
	"irq_ret", "sys_in", "sys_out", "abt", "abt_in", "abt_out",
	"abt_ret", "wfi", "fiq", "usr_ret", "sysc_ret"
};

/** @var char *hypcall_str[]
 * @brief readable syscall names
 */
char *hypcall_str[_HYP_CALLS] = {
	"syscall",
	"console",
	"yield",
	"getpage",
	"freepage",
	"irq_reques",
	"irq_enable",
	"irq_disabl",
	"irq_return",
	"exit",
	"cpuctrl",
	"syscall_request",
	"syscall_return",
	"pgfault_request",
	"undef_request",
	"enable_mmu",
	"setmode",
	"set_domain",
	"disable_cache",
	"reset",
	"idle",
	"flush_cache",
	"flush_icache",
	"flush_user_cache_range",
	"coherent_user_rang",
	"flush_kern_dcache_area",
	"dma_inv_range",
	"dma_clean_range",
	"dma_flush_range",
	"dchache_clean_area",
	"switch_mm",
	"set_pte",
	"tlb_flush",
	"enable_cache",
	"probe",
	"switch_task",
	"copy_page",
	"clear_page",
	"new_pgd",
	"set_pmd",
	"set_pte_ext",
	"get_pmdval",
	"get_timer",
	"io_open",
	"io_read",
	"io_write",
	"io_ioctl",
	"io_close",
	"event_send",
	"preempt_disable",
	"preempt_enable",
	"hyp",
	"abt_return",
	"usr_return",
	"get_tls",
	"cmpxchg",
	"cmpxchg64"
};

/** @fn void evt_hook_schedin(struct event *e)
 * @brief hook to handle scheduling in event
 * @param e the event structure to use
 *
 * @detailed
 * Save event timestamp to current domain
 * timestamp according to the current mode
 */
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

/** @fn void evt_hook_schedout(struct event *e)
 * @brief hook to handle scheduling in event
 * @param e the event structure to use
 *
 * @detailed
 * Calculate the schedule time and save the
 * time in the domain structure according to the current mode
 */
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

/** @fn void evt_hook_abtin(struct event *e)
 * @brief hook to handle entering the abort mode
 * @param e the event structure to handle
 */
void evt_hook_abtin(struct event *e)
{
	current->t_abtin = e->timestamp;
}

/** @fn void evt_hook_abtout(struct event *e)
 * @brief hook to handle leaving the abort mode
 * @param e the event structure to handle
 */
void evt_hook_abtout(struct event *e)
{
	adjust_time(&current->t_abt, &current->t_abtin, &e->timestamp);
}

/** @fn void evt_hook_irqin(struct event *e)
 * @brief hook to handle entering the irq mode
 * @param e the event structure to handle
 */
void evt_hook_irqin(struct event *e)
{
	current->t_irqin = e->timestamp;
}

/** @fn void evt_hook_irqout(struct event *e)
 * @brief hook to handle leave the irq mode
 * @param e the event structure to handle
 */
void evt_hook_irqout(struct event *e)
{
	adjust_time(&current->t_irq, &current->t_irqin, &e->timestamp);
}

/** @fn void evt_hook_irqret(struct event *e)
 * @brief hook to handle the end of the irq mode
 * @param e the event structure to handle
 */
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
/** @fn void evt_hook_wfi(struct event *e)
 * @brief hook to handle the idle (wfi) mode
 * @param e the event structure to handle
 */
void evt_hook_wfi(struct event *e)
{
	adjust_time(&current->t_irq, &current->t_irqin, &e->timestamp);
}

/** @fn void event_init(void)
 * @brief Initialisation of the event system
 */
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

/** @fn int event_new(int e)
 * @brief standard creation of a event
 * @param e the event number to create
 */
int event_new(int e)
{
	struct event event = {};

	time_get(&event.timestamp);
	event.event = e;
	event.nr = event_nr++;
	event.hypercall = current->hypercall;
	event.c_mode = current->mode;
	event.o_mode = current->old_mode;
	event.id = current->id;
	event.state = current->state;
	event.ctx = current->ctx_level;
	event.priv = sched->need_resched;

	ring_put(&trace_ring, &event, sizeof(event));

	if (evt_hook[e])
		evt_hook[e](&event);

	return 0;
}

/** @var char *mode_str[DMODE_SIZE]
 * @brief the readable modes
 */
char *mode_str[DMODE_SIZE] = { "INIT", "SVC", "IRQ", "USR", "ABT", "UND", "FIQ", "SYS", "HYP"};

/** @fn void event_show(struct event *event)
 * @brief print an event on the console
 * @param event a pointer to the event to print
 */
void event_show(struct event *event)
{
	printk("%12ld %5d %2d %2d %12s %2d %08s %08s\n",
		event->timestamp.tv_usec, event->nr,
		event->id, event->event,
		event_str[event->event], event->hypercall,
		mode_str[event->c_mode], mode_str[event->o_mode]);
}

/** @fn void event_dump(void)
 * @brief dump all events on the console
 */
void event_dump(void)
{
	struct event event;

	while (ring_get(&trace_ring, &event, sizeof(event)))
		event_show(&event);
}

void event_dump_last(int cnt)
{
	struct event event;
	unsigned int i = 1;
	char color[2];

	printk("%13s %6s %2s %2s %12s %2s %12s %08s %08s %3s %3s\n", "timestamp", "nr", "id", "ev", "event", "callnr", "hypercall", "mode", "omode", "ctx", "nrs");
	while (cnt--) {
	if (ring_get_entry(&trace_ring, &event, sizeof(event), i) < sizeof(event))
		return;
	i++;
	color[0] = '0' + event.id;
	color[1] = 0;
	printk(COLOR "%sm", color);
	printk("%6ld.%6ld %5d %2d %2d %12s %6d %12s %08s %08s %03d %03d\n",
		event.timestamp.tv_sec, event.timestamp.tv_usec,
		event.nr, event.id, event.event,
		event_str[event.event], event.hypercall,
		hypcall_str[event.hypercall],
		mode_str[event.c_mode], mode_str[event.o_mode], event.ctx, event.priv);
	}
	printk(COLOR_BLACK);
}

