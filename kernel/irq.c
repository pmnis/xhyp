/*
 * irq.c
 *
 * Interrupt handling
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
#include <xhyp/irq.h>
#include <xhyp/stdlib.h>
#include <xhyp/lowlevel.h>
#include <xhyp/domains.h>
#include <xhyp/hyp.h>
#include <xhyp/debug.h>
#include <xhyp/shared_page.h>
#include <xhyp/event.h>


void wfi(void)
{
	sched->need_resched = 1;
	event_new(EVT_WFI);
	_wfi();
}

#define NB_IRQ	32
int (*irq_table[NB_IRQ])(int, unsigned long);

/* IRQ init has yet not much to do */
void irq_init(void)
{
	pic_init();
}

/*
 * This is the hypervisor irq_request routine
 * irq: IRQ BIT provided
 */
int irq_request(int irq, int (*handler)(int, unsigned long))
{
	irq_table[irq] = handler;
	pic_enable(irq);
	return 0;
}

/*
 * Per domain virtual interrupt handling
 * irq: IRQ_MASK
 */
int ic = 0;
void send_irq(struct domain *d, unsigned long irq)
{
	struct shared_page *s;

	s = d->sp;
	if (d->state & (DSTATE_STOP|DSTATE_NONE|DSTATE_DEAD))
		return;

	if (d->type == DTYPE_HYP) return;	/* Hypervizor domain	*/

	debirq("d[%d] mode:%d: %08lx\n", d->id, d->mode, irq);

	if (d->mode == DMODE_IRQ) return;	/* Already in IRQ	*/
	if (!(s->v_irq_enabled & irq)) return;	/* IRQ not enabled	*/
	if (s->v_cpsr & dis_irqs) return;	/* SOC IRQS disabled	*/
	debirq("d[%d] E:%08lx M:%08lx\n", d->id, s->v_irq_enabled, s->v_irq_mask);

	/* update statistics	*/
	d->irq++;
	/* If the domain is sleeping, wake it up	*/
	debirq("[%d]: state 0x%08lx mode 0x%08lx\n", d->id, d->state, d->mode);
	if (d->state == DSTATE_SLEEP) {
		sched->add(d);
		d->state = DSTATE_READY;
		debirq("[%d]: state 0x%08lx mode 0x%08lx\n", d->id, d->state, d->mode);
	}
	/* mark the IRQ as pending in the shared page	*/
	s = d->sp;
	s->v_irq_pending |= irq;
	if (d->mode == DMODE_ABT) return;	/* do not interrupt abort */
	irq &= ~s->v_irq_mask;			/* clear masked IRQ	*/
	if (!irq) return;			/* deliver later	*/

	if (d == current) {
		/* Setup context for next slice		*/
		d->ctx = *_context;
	}
	/* The domain will transit to handle IRQ	*/
	mode_new(d, DMODE_IRQ);		/* New mode is DMODE_IRQ	*/
	if (d->id > 0) { debirq("d[%d] mode:%d: %08lx\n", d->id, d->mode, irq); }
//if (ic-- < 0 ) while(1);
	sched->need_resched++;
}

/*
 * Virtual interruptions for the domains
 */
void do_v_irq(unsigned long num)
{
	struct domain *d;
	int i;

	for (i = 0, d = domain_table; i < nb_domains; i++, d++)
		send_irq(d, 1 << num);
}

/*
 * The main IRQ routine called from asm(IRQ)->do_IRQ
 */
void do_irq(void)
{
	unsigned long mask;
	int irq = 0;
	struct domain *d;
	struct shared_page *s;
	int i;

	mask = pic_status();
	/* Put back current for scheduling */
	debirq("mask: %08lx\n", mask);
	while(mask) {
		if (mask & 0x01) {
			if (irq_table[irq]) {
				pic_clear(irq);
				irq_table[irq](irq, NULL);
				do_v_irq(irq);
				pic_enable(irq);
			} else {
				panic(_context, "Spurious Interrupt");
			}
		}
		mask = mask >> 1;
		irq++;
	}
	for (i = 1, d = &domain_table[i]; i < nb_usr_domains; i++, d++) {
		s = d->sp;
		if (s->v_irq_pending & s->v_irq_mask)
			sched->need_resched = 1;
	}
	if (sched->need_resched) {
		schedule();
	}
}

