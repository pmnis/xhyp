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
#include <xhyp/page_alloc.h>

#define write(fmt, ...) \
        do { sprintf(buffer, fmt, ## __VA_ARGS__); \
        _hyp_console(buffer, strlen(buffer)); } \
        while (0)


static unsigned long first;
struct page {
	unsigned long pfn;
	unsigned long flags;
};
static struct page *page_map;
static unsigned long page_max;
#define PAGE_INUSE	0x01

/*
** pgalloc_init: extect PFN start and PFN count
*/
int pgalloc_init(unsigned long start, unsigned long count)
{
	unsigned long size;
	unsigned long p;

	write("start %08lx count %08lx\n", start, count);

	size = count * sizeof(struct page);
	first = start + (size >> PAGE_SHIFT) + 1;
	page_map = (struct page *) (start << PAGE_SHIFT);
	page_max = count - (first - start);

	for (p = 0; p < page_max; p++) {
		page_map[p].flags = 0;
		page_map[p].pfn = first + p;
	}
	return 0;
}

unsigned long get_page(void)
{
	unsigned long p;

	for (p = 0; p < page_max; p++)
		if (page_map[p].flags != PAGE_INUSE) {
			page_map[p].flags |= PAGE_INUSE;
			return page_map[p].pfn << PAGE_SHIFT;
		}
	return 0;
}

void free_page(unsigned long addr)
{
	unsigned long p;

	p = addr >> PAGE_SHIFT;
	page_map[p].flags &= ~PAGE_INUSE;
}
