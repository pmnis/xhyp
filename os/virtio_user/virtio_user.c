#include <xhyp/shared_page.h>
#include <sys/xhyp.h>
//#include <xhyp/irq.h>
#include <xhyp/stdlib.h>
//#include <xhyp/domains.h>
#include <xhyp/hyp.h>
//#include <xhyp/event.h>

#include <xhyp/arinc.h>

#include <autoconf.h>

#include "hyp.h"

struct shared_page *xhyp_sp = (struct shared_page *) (0x02000000);

#define wait_next_period()		_hyp_idle()

#define MAX     10000000
static void delay(unsigned long max)
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
	_hyp_console("virtio user\n", 13);

	while(1) {
		_hyp_console("virtio_user\n", 13);
		delay(1000);
	}
}

