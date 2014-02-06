/*
 * sched.h
 *
 * scheduler definitions
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
#ifndef __SCHED_H
#define __SCHED_H

#include <xhyp/domains.h>
#include <xhyp/debug.h>
#include <xhyp/lowlevel.h>
#include <xhyp/soc.h>


struct xhyp_scheduler {
	int type;
	char name[32];
	int need_resched;
	int preempt_count;
	void (*add) (struct domain *);
	void (*add_from_sleep) (struct domain *);
	void (*add_to_sleep) (struct domain *);
	void (*delete) (struct domain *);
	void (*put) (struct domain *);
	int (*get) (void);
	void (*sleep) (struct domain *);
	void (*wake) (struct domain *);
	void (*stop) (struct domain *);
	void (*start) (struct domain *);
	void (*restart) (struct domain *);
	void (*kill) (struct domain *);
	void (*yield) (void);
	void (*slice) (void);
	void (*init) (void);
};
extern struct xhyp_scheduler *sched;

#define SCHED_ARINC	0x0001
#define SCHED_POSIX	0x0002
#define SCHED_EDF	0x0004
#define SCHED_IRQ	0x1000

#define exit()	sched->kill(current)
#endif
