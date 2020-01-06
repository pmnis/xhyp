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
int delay(unsigned long max)
{
	unsigned long end = xhyp_sp->jiffies + max;
	int elapsed;

	while (end > xhyp_sp->jiffies)
		elapsed++;
	return xhyp_sp->jiffies;
}


void irq_handler(unsigned long mask)
{
	unsigned long irq_mask = xhyp_sp->v_irq_pending;

	irq++;
	delay(20);
	xhyp_sp->v_irq_ack |= irq_mask ;
	_hyp_irq_return(0);
}

void start_kernel(void)
{
	int t;

	printf("This is the IRQ test program\n");
	printf("Request irq_handler\n");
	_hyp_irq_request(irq_handler, irq_stack + LSTACK_SIZE);

	_hyp_irq_enable(0x10);
	_hyp_irq_enable(0);
	while(1) {
		_hyp_irq_disable(0);
		printf("Irq dis: IRQ %d cpsr %02lx\n", irq, xhyp_sp->v_cpsr);
		t = delay(90);
		printf("%08d Irq dis: IRQ %d cpsr %02lx\n", t, irq, xhyp_sp->v_cpsr);
		_hyp_irq_enable(0);
		printf("Irq ena: IRQ %d cpsr %02lx\n", irq, xhyp_sp->v_cpsr);
		delay(90);
		printf("Irq ena: IRQ %d cpsr %02lx\n", irq, xhyp_sp->v_cpsr);
	}
	printf("----- FIN -----\n");
}
