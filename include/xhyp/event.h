/*
 * event.h
 *
 * event definitions
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
#ifndef __EVENT_H
#define __EVENT_H

#include <xhyp/timer.h>

struct event {
	struct timespec timestamp;
	int nr;
	int id;
	int event;
	int syscall;
	int state;
	int c_mode;
	int o_mode;
};

#define MAX_EVENT       1024

#define EVT_START	0
#define EVT_SCHEDIN	1
#define EVT_SCHEDOUT	2
#define EVT_IRQ		3
#define EVT_IRQIN	4
#define EVT_IRQOUT	5
#define EVT_IRQRET	6
#define EVT_SYSIN	7
#define EVT_SYSOUT	8
#define EVT_ABT		9
#define EVT_ABTIN	10
#define EVT_ABTOUT	11
#define EVT_ABTRET	12
#define EVT_WFI		13
#define EVT_NB		14

extern int event_new(int event);
extern void event_dump(void);
extern void event_init(void);

typedef void (*evt_hook_t)(struct event *);

#endif
