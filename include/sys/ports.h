/*
 * ports.h
 *
 * ports definitions
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
#ifndef __PORTS_H
#define __PORTS_H

#include <sys/fifo.h>

#define QPORT_SIZE	1024
#define QPORT_READY	0x0001
#define QPORT_OPEN	0x0002
#define QPORT_FROM_DRV	0x0004
#define QPORT_TO_DRV	0x0008
#define QPORT_DIR_MSK	0x000C
#define QPORT_OUT	0x0010
#define QPORT_IN	0x0020

struct queuing_port {
	struct fifo fifo;
	unsigned long flags;
	int remote;
	unsigned long buffer[QPORT_SIZE];
};

typedef unsigned long sampling_port_t;
typedef struct queuing_port queuing_port_t;

static inline void queuing_port_init(queuing_port_t *qp, unsigned long dir)
{
	fifo_init(&qp->fifo, QPORT_SIZE);
	if (dir & QPORT_OUT)
		qp->fifo.wp = qp->buffer;
	if (dir & QPORT_IN)
		qp->fifo.rp = qp->buffer;
	qp->flags = 0;
	qp->remote = 0;
}

extern int queuing_port_create(int, void *, unsigned int);
extern int queuing_port_send(int, void *, unsigned int);
extern int queuing_port_recv(int, void *, unsigned int);

#endif
