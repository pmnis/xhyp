#include <xhyp/page_alloc.h>

static struct page *page_map;
static unsigned long max_pfn;

/*
** pgalloc_init: expect PFN start and PFN count
*/
int pgalloc_init(unsigned long start, unsigned long count)
{
	int size, first;
	int i;

	size = count * sizeof(struct page);
	first = start + (size >> PAGE_SHIFT) + 1;
	page_map = (struct page *) (start << PAGE_SHIFT);
	max_pfn = count - (first - start);

	for (i = 0; i < max_pfn; i++) {
		page_map[i].flags = 0;
		page_map[i].pfn = first + i;
	}
	return 0;
}

unsigned long get_page(void)
{
	unsigned long i;

	for (i = 0; i < max_pfn; i++)
		if (!(page_map[i].flags & PAGE_INUSE)) {
			page_map[i].flags |= PAGE_INUSE;
			return page_map[i].pfn << PAGE_SHIFT;
		}
	return 0;
}

void free_page(unsigned long addr)
{
	unsigned long p = addr >> PAGE_SHIFT;

	page_map[p].flags &= ~PAGE_INUSE;
}

void free_pages(unsigned long first, int count)
{
	for (int i = 0; i < count; i++)
		page_map[i].flags &= ~PAGE_INUSE;
}

static int search_hole(int count)
{
	int first = 0;
	int next = 0;
	int i;

retry:
	for (i = 0; i < count && next < max_pfn; i++, next++) {
		if (page_map[next].flags & PAGE_INUSE)
			break;
	}

	if (next >= max_pfn)
		return -1;

	if (i < count) {
		first = ++next;
		goto retry;
	}

	return first;
}

unsigned long get_pages(int count)
{
	int p;
	int i;

	p = search_hole(count);
	if (p < 0)
		return p;

	for (i = 0; i < count; i++)
		page_map[p + i].flags |= PAGE_INUSE;

	return page_map[p].pfn << PAGE_SHIFT;
}

