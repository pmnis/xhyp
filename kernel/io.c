/*
 * io.c
 *
 * Gestion des IO
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

/** @file io.c
 * @brief implementation of queuing ports and standard IO
 */

/** @fn int queuing_port_create(int id, void *buf, unsigned int flags)
 * @brief creation of a queuing port
 * @param id the domain id to create the queuing ports to
 * @param buf a pointer to a buffer for the queuing_port data
 * @param flags to specify the way the queuing port is created
 * @return the qport id in case of success or a negative integer to indicate an error
 */
int queuing_port_create(int id, void *buf, unsigned int flags)
{
	int i;
	struct shared_page *s;
	struct queuing_port *qp;

	s = domain_table[id].sp;
	qp = s->qp;
	//deb_printf(DEB_INFO, "[--]: %08x\n", flags);
	for (i = 0; i < NB_QUEUING_PORT; i++, qp++) {
		//deb_printf(DEB_INFO, "[%02d]: %08x\n", i, qp->flags);
		if (!(qp->flags & QPORT_READY))
			continue;
		if ((qp->flags & QPORT_OPEN))
			continue;
		if (QPORT_DIR_MSK & qp->flags & flags)
			break;
	}
	if (i >= NB_QUEUING_PORT)
		return -1;

	qp->flags |= QPORT_OPEN;
	qp->remote = current->id;
	if (qp->flags & QPORT_TO_DRV)
		qp->fifo.wp = qp->buffer;
	if (qp->flags & QPORT_FROM_DRV)
		qp->fifo.rp = qp->buffer;
	//deb_printf(DEB_INFO, "Found port %d flags %08lx\n", i, qp->flags);

	return id << 16 | i ;
	
}
extern int queuing_port_send(int, void *, unsigned int);
extern int queuing_port_recv(int, void *, unsigned int);

/** @fn int hyp_io_open(void)
 * @brief standard IO open hypercall
 * @return the port id
 *
 * @detailed
 *   - R0 is the qport id to open
 *   - R1 are the open flags
 *
 *
 */
int hyp_io_open(void)
{
	unsigned long id;
	unsigned long flags;
	unsigned long f;
	struct domain *d;
	int i;

	id = CTX_arg0;
	flags = CTX_arg1;

	for (i = 0, d = domain_table; i < nb_domains; i++, d++) {
		if (d->type == DTYPE_DRV && d->d_drv_type == id)
			break;
	}
	if (i >= nb_domains)
		return -1;

	if (flags & HYP_IO_RDONLY)
		f = QPORT_FROM_DRV;
	else
		f = QPORT_TO_DRV;
	i = queuing_port_create(i, 0, f);

	return i;
}

int hyp_io_read(void)
{
	unsigned long id;
	unsigned long *ptr;
	unsigned long cnt;

	id = CTX_arg0;
	ptr = (unsigned long *)CTX_arg1;
	cnt = CTX_arg2;

	deb_printf(DEB_INFO, "id %d , ptr %p , cnt %d\n", id, ptr, cnt);

	return 0;
}

struct queuing_port *console_qp;
struct domain *console_domain;

int stdio_write(unsigned long fd, unsigned long buf, unsigned long cnt)
{
	int port = current->id * 2 + fd;
	struct queuing_port *qp;
	char *ptr;

	qp = &console_qp[port];
	ptr = (char *)virt_to_phys(current, buf);
	if (!paddr_in_domain(current, (unsigned long) ptr)) {
		debpanic("stdio_write Bad addr in r1: %08lx convert to %08lx\n", buf, ptr);
debpanic("d->base_addr %08lx XHYP_MEM_SIZE %08lx (d->offset << SECTION_SHIFT) %08lx\n", current->base_addr, XHYP_MEM_SIZE, (current->offset << SECTION_SHIFT));
		while(1);
	}
	cnt = fifo_put(&qp->fifo, (char *)ptr, cnt);
	return 0;

	debinfo("sending to console\n");
	send_irq(console_domain, IRQ_MASK_QPORT);

	return cnt;
}

int hyp_io_write(void)
{
	unsigned long id;
	char *ptr;
	unsigned long cnt;
	struct domain *d;
	struct shared_page *s;
	struct queuing_port *qp;
	int port;

	id = CTX_arg0 >> 16;
	if (!id)
		return stdio_write(CTX_arg0, CTX_arg1, CTX_arg2);

	debinfo("id: %08lx\n", CTX_arg0);
	port = CTX_arg0 & 0xFF;
	cnt = CTX_arg2;

	if (id >= nb_domains)
		panic(NULL, "Bad ID in r0\n");

	d = &domain_table[id];
	s = d->sp;
	qp = &s->qp[port];
	ptr = (char *)virt_to_phys(current, CTX_arg1);
	if (!paddr_in_domain(current, (unsigned long)ptr)) {
		deb_printf(DEB_INFO, "ptr %p\n", ptr);
		panic(NULL, "Bad addr in r1\n");
	}

	cnt = fifo_put(&qp->fifo, ptr, cnt);

	debinfo("sending to %d\n", d->id);
	send_irq(d, IRQ_MASK_QPORT);

	return cnt;
}

int hyp_io_ioctl(void)
{
	unsigned long id;
	unsigned long *ptr;
	unsigned long flags;
	unsigned long cmd;

	id = CTX_arg0;
	cmd = CTX_arg1;
	ptr = (unsigned long *) CTX_arg2;
	flags = CTX_arg3;

	deb_printf(DEB_INFO, "id %d , cmd %d , ptr %p , flags %08lx\n", id, cmd, ptr, flags);

	return 0;
}

int hyp_io_close(void)
{
	unsigned long fd;
	unsigned long flags;
	struct domain *d;
	int port;
	struct shared_page *s;
	struct queuing_port *qp;

	fd = CTX_arg0 >> 16;
	port = CTX_arg0 & 0x0F ;
	flags = CTX_arg1;

	deb_printf(DEB_INFO, "fd %08lx flags %08lx\n", fd, flags);

	d = &domain_table[fd];
	s = d->sp;
	qp = &s->qp[port];

	deb_printf(DEB_INFO, "domain: %d qpflags %08lx remote %d\n", d->id, qp->flags, qp->remote);

	queuing_port_init(qp, 0);

	return 0;
}

/*
 * console_init
 * 	setup the qport for the domains consoles
 * 	2 per domains 0 is stdin, 1 is stdout
 */
void console_init(void)
{
	struct domain *d;
	struct shared_page *s;
	struct queuing_port *qp;
	int i;

	for (i = 0, d = domain_table; i < nb_domains; i++, d++) {
		if (d->type == DTYPE_DRV && d->d_drv_type == HYP_IO_CONSOLE_ID)
			break;
	}

	s = d->sp;
	console_domain = d;
	qp = s->qp;
	console_qp = qp;
	for (i = 0; i < nb_domains; i++, qp++) {
		queuing_port_init(qp, QPORT_IN);
		qp->flags  = QPORT_READY|QPORT_FROM_DRV|QPORT_OPEN;
		qp->fifo.wp = (char *)phys_to_virt(d, (unsigned long)qp->buffer);
		qp->remote = i;
		qp++;
		queuing_port_init(qp, QPORT_OUT);
		qp->flags  = QPORT_READY|QPORT_TO_DRV|QPORT_OPEN;
		qp->fifo.rp = (char *)phys_to_virt(d, (unsigned long)qp->buffer);
		qp->remote = i;
	}

}
