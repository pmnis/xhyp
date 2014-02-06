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
void delay(unsigned long max);

void irq_handler(unsigned long mask)
{
	unsigned long irq_mask = xhyp_sp->v_irq_pending;

	//printk("%d irq mask: 0x%08x cpsr %02lx pending %08lx\n", irq, irq_mask, xhyp_sp->v_cpsr, xhyp_sp->v_irq_pending);
	//delay(90);
	irq++;
	xhyp_sp->v_irq_ack |= irq_mask ;
	//printk("Returning %d ack %08lx\n", irq, xhyp_sp->v_irq_ack);
	_hyp_irq_return(0);
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

unsigned long *p = 0;
void start_kernel(void)
{

	printf("This is the IRQ test program\n");
	printf("Try to erase memory at 0\n");
	//*p = 0;
	printf("Request irq_handler\n");
	_hyp_irq_request(irq_handler, irq_stack + LSTACK_SIZE);

	_hyp_irq_enable(0x10);
		//delay(90);
	_hyp_irq_enable(0);
	while(1) {
		_hyp_irq_disable(0);
		printf("A: IRQ %d cpsr %02lx\n", irq, xhyp_sp->v_cpsr);
		delay(90);
		_hyp_irq_enable(0);
		printf("B: IRQ %d cpsr %02lx\n", irq, xhyp_sp->v_cpsr);
		delay(90);
	}
	printf("----- FIN -----\n");
}
