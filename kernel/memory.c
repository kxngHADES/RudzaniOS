#include "memory.h"
#include "string.h"

#define PAGE_SIZE      4096
#define MEM_START      0x100000   /* Start managing from 1 MB */
#define MEM_END        0x1000000  /* Up to 16 MB */
#define TOTAL_PAGES    ((MEM_END - MEM_START) / PAGE_SIZE)
#define BITMAP_SIZE    (TOTAL_PAGES / 8)

/* Each bit represents a 4 KB page: 0 = free, 1 = used */
static uint8_t page_bitmap[BITMAP_SIZE];
static uint32_t used_pages = 0;

static void bitmap_set(uint32_t page) {
	page_bitmap[page / 8] |= (1 << (page % 8));
}

static void bitmap_clear(uint32_t page) {
	page_bitmap[page / 8] &= ~(1 << (page % 8));
}

static int bitmap_test(uint32_t page) {
	return (page_bitmap[page / 8] >> (page % 8)) & 1;
}

void mem_init(void) {
	memset(page_bitmap, 0, BITMAP_SIZE);
	used_pages = 0;
}

void *mem_alloc_page(void) {
	uint32_t i;
	for (i = 0; i < TOTAL_PAGES; i++) {
		if (!bitmap_test(i)) {
			bitmap_set(i);
			used_pages++;
			return (void *)(MEM_START + i * PAGE_SIZE);
		}
	}
	return (void *)0;  /* Out of memory */
}

void mem_free_page(void *page) {
	uint32_t addr = (uint32_t)page;
	if (addr < MEM_START || addr >= MEM_END) {
		return;
	}
	uint32_t index = (addr - MEM_START) / PAGE_SIZE;
	if (bitmap_test(index)) {
		bitmap_clear(index);
		used_pages--;
	}
}

uint32_t mem_get_total_pages(void) {
	return TOTAL_PAGES;
}

uint32_t mem_get_free_pages(void) {
	return TOTAL_PAGES - used_pages;
}

uint32_t mem_get_used_pages(void) {
	return used_pages;
}
