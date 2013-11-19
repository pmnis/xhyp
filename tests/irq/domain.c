#include <sys/shared_page.h>
#include <sys/xhyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/domains.h>
#include <xhyp/hyp.h>
#include <xhyp/event.h>
#include <xhyp/vpages.h>

#include <autoconf.h>

struct shared_page *xhyp_sp = (struct shared_page *) (0x02000000);

#define LSTACK_SIZE     1024
static unsigned long irq_stack[LSTACK_SIZE];

volatile int irq = 0;

void irq_handler(unsigned long irq_mask)
{
	printk("irq mask: 0x%08x cpsr %02lx\n", irq_mask, xhyp_sp->v_cpsr);
	irq++;
	_hyp_irq_return();
}

#define MAX	10000000
void delay(unsigned long max)
{
	int i = 0;
	int j = 0;
	int k = 0;

	while (i++ < max)
		while (j++ < MAX)
			while (k++ < MAX);
}

void start_kernel(void)
{

	printf("This is the IRQ test program\n");
	printf("Request irq_handler\n");
	_hyp_irq_request(irq_handler, irq_stack + LSTACK_SIZE);

	_hyp_irq_enable(0x10);
	_hyp_irq_enable(0);
	while(1) {
		xhyp_sp->v_cpsr |= 0xc0;
		printf("IRQ %d cpsr %02lx\n", irq, xhyp_sp->v_cpsr);
		delay(9000);
		_hyp_irq_enable(0);
		printf("IRQ %d cpsr %02lx\n", irq, xhyp_sp->v_cpsr);
		delay(9000);
	}
	printf("----- FIN -----\n");
}
