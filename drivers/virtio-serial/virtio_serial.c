#include <xhyp/config.h>
#include <xhyp/types.h>
#include <xhyp/shared_page.h>
#include <sys/xhyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/hyp.h>
#include <xhyp/delay.h>
#include <xhyp/virtio_mmio.h>

#include <autoconf.h>

unsigned long periph_base = PERIPH_BASE;

struct virtio_registers	*virtio_p = (struct virtio_registers *) (PERIPH_BASE + VIRTIO_MMIO_BASE);

struct shared_page *xhyp_sp = (struct shared_page *) (0x02000000);

#define wait_next_period()		_hyp_idle()

char buffer[80];
#define write(fmt, ...) \
	do { sprintf(buffer, fmt, ## __VA_ARGS__); \
	_hyp_console(buffer, strlen(buffer)); } \
	while (0)

int serial_write(const void *s, int n)
{
	snprintf(buffer, n, s);
	buffer[n] = 0;
	_hyp_console(buffer, strlen(buffer));
	return 0;
}

static void virtio_mmio_probe(int devnum)
{
	struct virtio_registers *dev_p = &virtio_p[devnum];

	write("virtio_mmio_probe: %d probe at %08x\n", devnum, dev_p);
	if (dev_p->magic == VIRTIO_MMIO_MAGIC)
		write("Virtio magic\n");
	else
		write("Unknown\n");
}

static void virtio_mmio_init(void)
{
	int i;

	for (i = 0; i < VIRTIO_MMIO_DEV_MAX; i++)
		virtio_mmio_probe(i);
}

void start_kernel(void)
{
	write("virtio_serial %08x\n", &virtio_p->magic);
	virtio_mmio_init();

	while(1) {
		write("virtio_serial %08x\n", virtio_p->magic);
		delay(1000);
	}
}

