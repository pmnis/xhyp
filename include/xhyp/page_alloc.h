
#define PAGE_SHIFT 12

int pgalloc_init(unsigned long start, unsigned long count);
unsigned long get_page(void);
void free_page(unsigned long addr);

#define NULL (void *)0UL
