#include <autoconf.h>
#include <xhyp/config.h>
#include <xhyp/types.h>
#include <xhyp/shared_page.h>
#include <sys/xhyp.h>
#include <xhyp/stdlib.h>
#include <xhyp/hyp.h>
#include <xhyp/delay.h>
#include <xhyp/virtio_mmio.h>
#include <xhyp/virtio.h>
#include <sys/virtio.h>
#include <xhyp/irq.h>
#include <xhyp/page_alloc.h>
#include <xhyp/virtio_console.h>


unsigned long periph_base = PERIPH_BASE;

struct virtio_registers	*virtio_p = (struct virtio_registers *) (PERIPH_BASE + VIRTIO_MMIO_BASE);

struct shared_page *xhyp_sp = (struct shared_page *) (0x02000000);

#define VIRTIO_CONSOLE_F_DEFAULT 0x19000006
#define VIRTIODEV_F_INIT	0x01
struct virtio_device {
	struct virtio_registers *regs;
	struct virtio_console_config *config; 
	u64 features;
	unsigned int flags;
	int id;
	u32 *emergency;
	int nb_ports;
	struct vring vring;
};
#define VIRTIO_MAX_DEVICES 3
struct virtio_device virtio_device[VIRTIO_MAX_DEVICES];

#define IRQ_STACK_SIZE  1024
static unsigned long irq_stack[IRQ_STACK_SIZE];

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

char *feature_str[64] = {
};
void init_feature_str(void)
{
	int i;

	for (i = 0; i < 63; i++)
		feature_str[i] = "UNKNOWN";

	feature_str[0] = "VIRTIO_CONSOLE_F_SIZE";
	feature_str[1] = "VIRTIO_CONSOLE_F_MULTIPORT";
	feature_str[2] = "VIRTIO_CONSOLE_F_EMERG_WRITE";
	feature_str[24] = "VIRTIO_F_NOTIFY_ON_EMPTY";
	feature_str[27] = "VIRTIO_F_ANY_LAYOUT";
	feature_str[28] = "VIRTIO_TRANSPORT_F_START";
	feature_str[32] = "VIRTIO_F_VERSION_1";
	feature_str[33] = "VIRTIO_F_IOMMU_PLATFORM";
	feature_str[34] = "VIRTIO_F_RING_PACKED";
	feature_str[36] = "VIRTIO_F_ORDER_PLATFORM";
	feature_str[37] = "VIRTIO_F_SR_IOV";
	feature_str[38] = "VIRTIO_TRANSPORT_F_END";
}

void show_features(u64 features)
{
	int i;

	for (i = 0; i < 63; i++) {
		if (features & (1ULL << 63ULL))
			write("feature[%02d]: %s\n", 63 - i, feature_str[63 - i]);
		features <<= 1;
	}
}

int virtio_mmio_init_queues(struct virtio_device *dev)
{
	struct virtio_registers *vregs = dev->regs;
	int max;
	unsigned long *p;

	write("nb queues: %d\n", dev->nb_ports);
	*dev->emergency = 0x61626364;

	vregs->queue_sel = 0;
	if (vregs->queue_ready != 0) {
		write("Queue not ready\n");
		return -1;
	}

	max = vregs->queue_num_max;
	write("queue_num_max: %d\n", max);
	p = (unsigned long *)get_page();
	memset(p, 0, 4096);

	vring_init(&dev->vring, XHYP_QUEUE_SIZE, p, 16);

	vregs->queue_desc_low = ((unsigned long)dev->vring.desc + xhyp_sp->prefix) & 0xffffffff ;
	vregs->queue_desc_high = 0;
	vregs->queue_avail_low = ((unsigned long)dev->vring.avail + xhyp_sp->prefix) & 0xffffffff ;
	vregs->queue_avail_high = 0;
	vregs->queue_used_low = ((unsigned long)dev->vring.used + xhyp_sp->prefix) & 0xffffffff ;
	vregs->queue_used_high = 0;
	vregs->queue_num = dev->vring.num;
	vregs->queue_ready = 0x01;
	return 0;
}

static void virtio_irq(unsigned long msk)
{
	unsigned long mask = xhyp_sp->v_irq_pending;

	xhyp_sp->v_irq_pending = 0;
	if (!(mask & IRQ_MASK_VIRTIO))
		goto end;

	write("IRQ VIRTIO\n");
end:
	/* acknowledge the irq  */
	xhyp_sp->v_irq_ack = mask;
	/* Return from interrupt	*/
	_hyp_irq_return(0);
}

void virtio_configure(struct virtio_device *dev)
{
	struct virtio_console_config *conf;

	conf = (struct virtio_console_config *) dev->regs->config_space;

	if (dev->features & VIRTIO_CONSOLE_F_EMERG_WRITE)
		dev->emergency = &conf->emerg_wr;

	if (dev->features & VIRTIO_CONSOLE_F_MULTIPORT)
		dev->nb_ports = conf->max_nr_ports;
}

struct virtio_device * virtio_dev_alloc(struct virtio_registers *vregs)
{
	struct virtio_device *dev = &virtio_device[0];
	int i;

	for (i = 0; i < VIRTIO_MAX_DEVICES; i++, dev++) {
		if (dev->flags & VIRTIODEV_F_INIT)
			continue;
		dev->regs = vregs;
		dev->flags |= VIRTIODEV_F_INIT;
		dev->id = i;
		return dev;
	}

	return NULL;
}

void virtio_negociate_features(struct virtio_device *dev)
{
	struct virtio_registers *vregs = dev->regs;

	/* Features */
	vregs->host_features_sel = 1;
	dev->features = vregs->host_features;
	write("Virtio[%d]: host_features_0: 0x%08x\n", dev->id, vregs->host_features);
	dev->features <<= 32;

	vregs->host_features_sel = 0;
	dev->features |= vregs->host_features;
	write("Virtio[%d]: host_features_0: 0x%08x\n", dev->id, vregs->host_features);

	show_features(dev->features);
	/* Get configuration */

	/* accept known features */
	vregs->guest_features_sel = 1;
	vregs->guest_features = 0; /* We do not know any feature so high */

	vregs->guest_features_sel = 0;
	vregs->guest_features = (u32)(dev->features) & VIRTIO_CONSOLE_F_DEFAULT;

}

static void virtio_mmio_probe(int devnum)
{
	struct virtio_device *dev;
	struct virtio_registers *vregs = &virtio_p[devnum];

	if (vregs->device_id == 0)
		return;

	if (vregs->magic != VIRTIO_MMIO_MAGIC) {
		write("Virtio: bad magic\n");
		return;
	}

	switch (vregs->version) {
	case 1:
		write("Virtio: Version 1: Legacy interface\n");
		break;
	case 2:
		write("Virtio: Version 1.1: New interface\n");
		write("Unsupported\n");
		return;
	default:
		write("Virtio: Unsupported version number %d\n", vregs->version);
		return;
	}

	dev = virtio_dev_alloc(vregs);
	if (!dev)
		return;
	virtio_negociate_features(dev);

	/* get configuration */
	virtio_configure(dev);

	vregs->status = VIRTIO_CONFIG_S_ACKNOWLEDGE | VIRTIO_CONFIG_S_DRIVER;

	vregs->status |= VIRTIO_CONFIG_S_FEATURES_OK;
	/* read status again */
	if (!(vregs->status & VIRTIO_CONFIG_S_FEATURES_OK)) {
		write("Virtio[%d]: bad status     : 0x%08x\n", devnum, vregs->status);
		return;
	}

	/* Device specific setup.... virtqueues .... */

	if (virtio_mmio_init_queues(dev)) {
		write("Virtio[%d]: status	 : 0x%08x\n", devnum, vregs->status);
		return;
	}

	/* Request VIRTIO IRQ */
	_hyp_irq_request(virtio_irq, irq_stack + STACK_SIZE);
	_hyp_irq_enable(IRQ_MASK_TIMER);

	/* status */
	vregs->status |= VIRTIO_CONFIG_S_DRIVER_OK;
	write("Virtio[%d]: status	 : 0x%08x\n", devnum, vregs->status);
}

static void virtio_mmio_init(void)
{
	int i;

	for (i = 0; i < VIRTIO_MMIO_DEV_MAX; i++)
		virtio_mmio_probe(i);
}

extern void *alloc_start;
static void memory_init(void)
{
	unsigned long start, count;

	start = (((unsigned long) &alloc_start) >> PAGE_SHIFT) + 1;
	count = (xhyp_sp->mem_end >> PAGE_SHIFT) - start;

	pgalloc_init(start, count);
}

void start_kernel(void)
{
	memory_init();
	init_feature_str();
	virtio_mmio_init();

	while(1) {
		//write("virtio_serial %08x\n", virtio_p->magic);
		delay(1000);
	}
}

