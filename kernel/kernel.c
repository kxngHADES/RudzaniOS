#include "vga.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "memory.h"
#include "paging.h"
#include "kheap.h"
#include "fs.h"
#include "initrd.h"
#include "shell.h"
#include "process.h" // Added process.h

void kernel_main(void) {
	/* Initialize VGA text-mode display */
	vga_init();

	/* Print boot banner */
	vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
	vga_print_line("========================================");
	vga_print_line("         RudzaniOS v0.1.0");
	vga_print_line("    A Minimal x86 Operating System");
	vga_print_line("========================================");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
	vga_print_line("");

	/* Initialize IDT and interrupt system */
	vga_print("[INIT] Interrupt Descriptor Table... ");
	idt_init();
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line("OK");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Initialize PIT timer at 100 Hz */
	vga_print("[INIT] Programmable Interval Timer... ");
	timer_init(100);
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line("OK");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Initialize keyboard driver */
	vga_print("[INIT] PS/2 Keyboard Driver...        ");
	keyboard_init();
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line("OK");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Initialize physical memory manager */
	vga_print("[INIT] Physical Memory Manager...     ");
	mem_init();
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line("OK");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Initialize paging */
	vga_print("[INIT] x86 Paging (Identity 4MB)...   ");
	paging_init();
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line("OK");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Initialize kernel heap */
	vga_print("[INIT] Kernel Heap (Placement)...     ");
	kheap_init();
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line("OK");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Test heap allocation */
	vga_print("[TEST] Allocating 1KB on heap...      ");
	void *test_ptr = kmalloc(1024);
	if (test_ptr) {
		vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
		vga_print("OK ");
		vga_print_hex((uint32_t)test_ptr);
		vga_print_line("");
	} else {
		vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
		vga_print_line("FAILED");
	}
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Initialize filesystem */
	vga_print("[INIT] Initial Ramdisk (Initrd)...    ");
	fs_root = initialise_initrd();
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	vga_print_line("OK");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	vga_print_line("");
	vga_set_color(VGA_YELLOW, VGA_BLACK);
	vga_print_line("All systems initialized. Filesystem ready.");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
	vga_print_line("");

	/* Start the interactive shell */
	shell_run();
}
