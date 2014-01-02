/*
 * main.c
 *
 * main routine
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

#include <xhyp/types.h>
#include <xhyp/config.h>
#include <xhyp/lowlevel.h>
#include <xhyp/mmu.h>
#include <xhyp/stdlib.h>
#include <xhyp/domains.h>
#include <xhyp/irq.h>
#include <xhyp/timer.h>
#include <xhyp/mm.h>
#include <xhyp/debug.h>
#include <xhyp/setup.h>
#include <xhyp/hyp.h>
#include <xhyp/sched.h>
#include <xhyp/event.h>
#include <xhyp/serial.h>

#include <sys/io.h>


unsigned long periph_base = PERIPH_BASE;

void show_tags(struct tag *tag)
{
	int i;

	if (tag == NULL)
		return;

	for (i = 0; i < 10; i++) {
		printf("tag: %08lx\n", tag[i].hdr.tag);
		switch(tag[i].hdr.tag) {
		case ATAG_CMDLINE:
			printf("cmdline: %s\n", tag[i].u.cmdline.cmdline);
			break;
		default:
			break;
		case 0:
			return;
		}
	}
}

/*
 * function: start_xhyp
 * parameters:
 * 	- r0, r1 and r2 from SOC
 * purpose:
 * 	called from assembly on RESET
 * 	do main hardware initializations
 */
void show_it(void)
{
        unsigned long *p = (unsigned long *)domain_table[4].tbl_l1;
        debinfo("coarse at %p: %08lx\n", p, *p);
        p = domain_table[4].tbl_l2;
        debinfo("coarse at %p: %08lx\n", p, *p);
        p++;
        debinfo("coarse at %p: %08lx\n", p, *p);
}

void start_xhyp (unsigned long r0, unsigned long cpuid, struct tag *tagp)
{
	unsigned long r;

	if (XHYP_MEM_SIZE < (XHYP_CORE_SIZE + XHYP_TBL_SIZE)) {
		panic(NULL, "XHYP size too small\n");
	}
	/* Init trace system */
	event_init();
	/* Init the irq controller first before MMU init */
	irq_init();
	/* setup serial */
	serial_init();

	/* Usefull informations and tests */
	printk("Starting hypervizor x-hyp\n");
	printk("%s - %s\n", VERSION_STRING, COPYRIGHTS_STRING);

	show_tags(tagp);

	r = _get_c0_ID();
	printk("Processor : 0x%08x\n", r);
	r = _get_c0_CT();
	printk("Cache type: 0x%08x\n", r);
	r = _get_c0_TCM();
	printk("TCM Status: 0x%08x\n", r);

	debdeb("Hypervisor ends at 0x%08x\n", &__hyp_end);
	debdeb("Hypervisor alloc   0x%08x\n", XHYP_MEM_SIZE);

	/* Setup XHYP software Page table	*/
	init_page_table();
	/* Allocate XHYP hardware page table	*/
	alloc_page_tables(&domain_table[0]);
	/* Setup XHYP page table as flat	*/
	prepare_mmu_flat();
	_setup_domain(0xffffffff);	/* hyp control all memory	*/

	/* Initialize MMU translation	*/
	setup_mmu();
	/* Init the console		*/
	serial_init();
	/* Init the irq controller	*/
	irq_init();
	/* Init the timer		*/
	timer_init();
	printk("timer freq %dHz\n", HZ);
	timer_periodic_ms(TIMER_PERIOD);
	/* Initialize the scheduler	*/
	printk("sched init\n");
	sched->init();
	printk("x-hyp startup process ok\n");
	/* prepare the domains		*/
	setup_domains();
	/* prepare consoles		*/
	console_init();

	/* Start the first domain	*/
	printk("x-hyp scheduler: %s\n", sched->name);
	wfi();


	/* Should never been reached	*/
	deb_printf(DEB_PANIC, "System idle, cpsr: 0x%08x\n", _get_cpsr());
	while(1)
		;
}


