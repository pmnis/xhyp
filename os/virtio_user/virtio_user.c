#include <sys/shared_page.h>
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

void start_kernel(void)
{
	_hyp_console("virtio user\n", 12);

	while(1) {
		_hyp_console("x-hyp: ", 12);
		wait_next_period();
	}
}

