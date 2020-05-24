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

unsigned long get_pages(int count)
{
	unsigned long p, q;
	int i;

	for (i = 0; i < count; i++) {
		p = get_page();
		if (!q)
			q = p;
		if (!p)
			return 0;
	}
	return q;
}

