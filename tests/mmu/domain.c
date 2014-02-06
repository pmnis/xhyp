#include <sys/shared_page.h>
#include <sys/xhyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/domains.h>
#include <xhyp/hyp.h>
#include <xhyp/event.h>
#include <xhyp/vpages.h>

#include <autoconf.h>

struct shared_page *xhyp_sp = (struct shared_page *) (0x02000000);

#define TEST_TYPE	2

#define LSTACK_SIZE     1024
static unsigned long abt_stack[LSTACK_SIZE];

void pgfault_handler(void *addr, unsigned long flags, unsigned long fsr)
{
	printk("page fault at 0x%08x flags 0x%08lx fsr 0x%08x\n", addr, flags, fsr);
	_hyp_exit(1);
}

#define L1_TABLE_SIZE   1 << 12
unsigned long tbl_l1 [VPGD_SIZE] __attribute__((__aligned__(1<<14)));

void build_tbl1(void)
{
	int i;
	int first;
	unsigned long *p;

	printk("table at %p size %08lx\n", tbl_l1, sizeof(tbl_l1));

	for (i = 0, p = tbl_l1; i < VPGD_SIZE; i++, p++) {
		*p = 0;
	}
	first = (unsigned long)(xhyp_sp) >> 20;
	printk("first entry: %08lx\n", first);
	p = &tbl_l1[first];
	for (i = 0; i < 32; i++, p++) {
		*p = ((first + i) << 20);
		*p |= VSEC_DEFAULT;
		if (!i) printk("[%02d]", i);
		else if (! (i % 8)) printk("\n[%02d]", i);
		printk(" %08lx", *p);
	}
	printk("\n");
}

void start_kernel(void)
{

	printf("This is the MMU test program\n");
	printf("Request pgfault_handler\n");
	_hyp_pgfault_request(pgfault_handler, abt_stack + LSTACK_SIZE);

#if TEST_TYPE == 1
	{
	char *p = 0;

	printf("Do a page fault by accessing 0x0\n");
	*p = 1;
	printf("This is an error\n");
	}
#endif
	build_tbl1();
	printf("Set new PGD\n");
	_hyp_new_pgd((unsigned long)tbl_l1);
	printf("Set new PMD entry at offset 0x033\n");
	printf("----- FIN -----\n");
}
