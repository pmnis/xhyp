#include <xhyp/config.h>
#include <xhyp/shared_page.h>
#include <sys/xhyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/hyp.h>

#include <autoconf.h>

unsigned long periph_base = PERIPH_BASE;

#define VIRTIO_00_OFFSET	0x00208000
struct virtio_registers {
	unsigned int reg0;
	unsigned int reg1;
	unsigned int reg2;
	unsigned int reg3;
	unsigned int reg4;
};

struct virtio_registers	*virtio_p = (struct virtio_registers *) (PERIPH_BASE + VIRTIO_00_OFFSET);

struct shared_page *xhyp_sp = (struct shared_page *) (0x02000000);

#define wait_next_period()		_hyp_idle()

char buffer[80];
#define write(fmt, ...) \
	sprintf(buffer, fmt, ## __VA_ARGS__); \
	_hyp_console(buffer, strlen(buffer));

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

int serial_write(const char *s, int cnt)
{
	return 0;
}

void start_kernel(void)
{
	write("virtio_serial %08x\n", &virtio_p->reg0);

	while(1) {
		write("virtio_serial %08x\n", virtio_p->reg0);
		delay(1000);
	}
}

