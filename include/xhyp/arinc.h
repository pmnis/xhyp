/*
 * arinc.h
 *
 * arinc definitions
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
#ifndef __ARINC_H
#define __ARINC_H

#include <xhyp/domains.h>
#include <xhyp/debug.h>
#include <xhyp/lowlevel.h>
#include <xhyp/soc.h>

#define SCHED_ARINC_MAX_FRAMES	64


struct minor_frame {
	int slot_id;
	int slot_start;
	int slot_size;
	int dom_id;
};

struct major_frame {
	int     minor_count;
	int	frame_start;
	int	frame_period;
	int	frame_size;
	struct minor_frame minor[SCHED_ARINC_MAX_FRAMES];
};

extern struct major_frame *major;

#endif
