#include "paging.h"
#include "memory.h"
#include "string.h"
#include "vga.h"

/* 1024 entries per directory/table for 4GB total space */
static uint32_t kernel_page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));
static uint32_t heap_page_table[1024] __attribute__((aligned(4096)));

void paging_init(void) {
	uint32_t i;

	/* Initialize page directory with empty entries */
	for (i = 0; i < 1024; i++) {
		kernel_page_directory[i] = 0 | PAGE_WRITABLE; // Not present
	}

	/* Identity map the first 4MB (0x00000000 to 0x003FFFFF) */
	/* This covers BIOS, Bootloader, Kernel at 0x10000, and VGA at 0xB8000 */
	for (i = 0; i < 1024; i++) {
		first_page_table[i] = (i * 4096) | PAGE_PRESENT | PAGE_WRITABLE;
	}

	/* Map the 16MB-20MB range for the kernel heap */
	/* 16MB is index 4 in the page directory (16MB / 4MB = 4) */
	for (i = 0; i < 1024; i++) {
		heap_page_table[i] = (0x1000000 + i * 4096) | PAGE_PRESENT | PAGE_WRITABLE;
	}

	/* Put page tables into the page directory */
	kernel_page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_WRITABLE;
	kernel_page_directory[4] = ((uint32_t)heap_page_table) | PAGE_PRESENT | PAGE_WRITABLE;

	/* Load directory and enable paging */
	paging_switch_directory(kernel_page_directory);
	paging_enable();
}

void paging_switch_directory(uint32_t *directory) {
	__asm__ volatile("mov %0, %%cr3" : : "r"(directory));
}

void paging_enable(void) {
	uint32_t cr0;
	__asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000001; // Enable PG (bit 31) and PE (bit 0)
	__asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
}
