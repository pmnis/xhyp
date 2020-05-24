
#define PAGE_SHIFT 12

int pgalloc_init(unsigned long start, unsigned long count);
unsigned long get_page(void);
void free_page(unsigned long addr);
unsigned long get_pages(int count);

#define NULL (void *)0UL

#define PAGE_OFFSET 0x02000000
#define virt_to_phys(v) (v - PAGE_OFFSET)
#define virt_to_bus(v) (virt_to_phys(v) + xhyp_sp->prefix)

struct page {
        unsigned long pfn;
#define PAGE_INUSE      0x01
        unsigned long flags;
};

