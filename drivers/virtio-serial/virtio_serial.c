#include <xhyp/config.h>
#include <xhyp/types.h>
#include <xhyp/shared_page.h>
#include <sys/xhyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/hyp.h>
#include <xhyp/delay.h>
#include <xhyp/virtio_mmio.h>
#include <xhyp/virtio.h>
#include <xhyp/irq.h>

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

int virtio_mmio_init_queues(int evnum)
{
	return -1;
}

static void virtio_mmio_probe(int devnum)
{
	u64 features;
	struct virtio_registers *dev_p = &virtio_p[devnum];

	write("virtio_mmio_probe: %d probe at %08x\n", devnum, dev_p);
	if (dev_p->magic != VIRTIO_MMIO_MAGIC) {
		write("Virtio: bad magic\n");
		return;
	}
	write("virtio_mmio_probe: %d version: %d\n", devnum, dev_p->version);
	switch (dev_p->version) {
	case 1:
		write("Virtio: Version 1: Legacy interface\n");
		break;
	case 2:
		write("Virtio: Version 1.1: New interface\n");
		write("Unsupported\n");
		return;
	default:
		write("Virtio: Unsupported version number %d\n", dev_p->version);
		return;
	}
	if (dev_p->device_id == 0) {
		write("Virtio[%d]: no device\n", devnum);
		return;
	}
	write("Virtio[%d]: device: 0x%08x\n", devnum, dev_p->device_id);
	write("Virtio[%d]: vendor: 0x%08x\n", devnum, dev_p->vendor_id);

	dev_p->status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;
	/* Features */
	dev_p->host_features_sel = 1;
	features = dev_p->host_features;
	write("Virtio[%d]: host_features_0: 0x%08x\n", devnum, dev_p->host_features);
	features <<= 32;

	dev_p->host_features_sel = 0;
	features |= dev_p->host_features;
	write("Virtio[%d]: host_features_0: 0x%08x\n", devnum, dev_p->host_features);

	/* accept all features */
	features = dev_p->host_features;

	dev_p->guest_features_sel = 1;
	dev_p->guest_features = (u32)(features >> 32);

	dev_p->guest_features_sel = 0;
	dev_p->guest_features = (u32)features;

	dev_p->status |= VIRTIO_CONFIG_S_FEATURES_OK;
	/* read status again */
	if (!(dev_p->status & VIRTIO_CONFIG_S_FEATURES_OK)) {
		write("Virtio[%d]: bad status     : 0x%08x\n", devnum, dev_p->status);
		return;
	}

	/* Device specific setup.... virtqueues .... */

	if (virtio_mmio_init_queues(devnum)) {
		write("Virtio[%d]: status	 : 0x%08x\n", devnum, dev_p->status);
		return;
	}

	write("Virtio[%d]: guestfeatures_0: 0x%08x\n", devnum, dev_p->guest_features);
	write("Virtio[%d]: guestfeatures_1: 0x%08x\n", devnum, dev_p->guest_features_sel);
	/* status */
	dev_p->status |= VIRTIO_CONFIG_S_DRIVER_OK;
	write("Virtio[%d]: status	 : 0x%08x\n", devnum, dev_p->status);
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
		//write("virtio_serial %08x\n", virtio_p->magic);
		delay(1000);
	}
}

