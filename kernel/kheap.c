#include "kheap.h"
#include "memory.h"
#include "vga.h"

#define KHEAP_START 0x1000000 /* 16 MB */
#define KHEAP_MAX   0x2000000 /* 32 MB */

static uint32_t placement_address = KHEAP_START;

void kheap_init(void) {
	/* Basic placement allocator for now */
	placement_address = KHEAP_START;
}

void *kmalloc(size_t size) {
	/* 
	 * Extremely simple placement allocator. 
	 * Doesn't support free() yet, but works for permanent kernel allocations.
	 */
	uint32_t addr = placement_address;
	placement_address += size;

	/* Align to 4 bytes */
	if (placement_address & 0x03) {
		placement_address &= 0xFFFFFFFC;
		placement_address += 4;
	}

	if (placement_address >= KHEAP_MAX) {
		vga_print_line("KERNEL KERNEL PANIC: OUT OF HEAP MEMORY!");
		for(;;);
	}

	return (void *)addr;
}

void kfree(void *ptr) {
	(void)ptr;
	/* Not implemented in simple placement allocator */
}
