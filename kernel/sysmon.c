#include "sysmon.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "memory.h"
#include "string.h"
#include "process.h"
#include <stdint.h>

static void cpuid(uint32_t eax, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
	__asm__ volatile ("cpuid"
		: "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
		: "a" (eax));
}

void sysmon_start(void) {
	uint32_t pid = process_register("sysmon");
	vga_clear();
	
	uint32_t eax, ebx, ecx, edx;
	
	/* Get CPU Vendor */
	char vendor[13];
	cpuid(0, &eax, &ebx, &ecx, &edx);
	*(uint32_t*)&vendor[0] = ebx;
	*(uint32_t*)&vendor[4] = edx;
	*(uint32_t*)&vendor[8] = ecx;
	vendor[12] = '\0';

	/* Get CPU Brand String */
	char brand[49] = "Unknown CPU";
	cpuid(0x80000000, &eax, &ebx, &ecx, &edx);
	if (eax >= 0x80000004) {
		uint32_t *p = (uint32_t*)brand;
		cpuid(0x80000002, p, p+1, p+2, p+3);
		cpuid(0x80000003, p+4, p+5, p+6, p+7);
		cpuid(0x80000004, p+8, p+9, p+10, p+11);
		brand[48] = '\0';
	}

	uint32_t last_tick = 0;
	while (1) {
		if (keyboard_has_input()) {
			char k = keyboard_getchar();
			if (k == 'q' || k == 'Q' || k == 24) { /* 24 = Ctrl+X */
				break;
			}
		}
		
		uint32_t current_tick = timer_get_ticks();
		if (current_tick - last_tick >= 50 || last_tick == 0) { /* Update twice a second */
			last_tick = current_tick;
			
			vga_set_cursor(0, 0);
			vga_set_color(VGA_LIGHT_CYAN, VGA_BLUE);
			vga_print("================================================================================");
			vga_print("  RudzaniOS System Monitor (sysmon)                                             ");
			vga_print("================================================================================");
			
			vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
			vga_print("CPU Information:                                                                ");
			
			vga_print("  Vendor: "); vga_print(vendor);
			vga_print("                                                                \n"); /* Flush line with spaces */
			
			vga_print("  Brand:  "); vga_print(brand);
			vga_print("                                                                \n");
			vga_print("                                                                                ");
			
			uint32_t total = mem_get_total_pages() * 4;
			uint32_t used = mem_get_used_pages() * 4;
			uint32_t free = mem_get_free_pages() * 4;
			
			vga_print("Memory Information:                                                             ");
			vga_print("  Total RAM Capacity: "); vga_print_dec(total); vga_print(" KB                                  \n");
			vga_print("  Used RAM:           "); vga_print_dec(used); vga_print(" KB                                  \n");
			vga_print("  Free RAM:           "); vga_print_dec(free); vga_print(" KB                                  \n");
			vga_print("                                                                                ");
			
			uint32_t uptime = current_tick / 100;
			vga_print("System Uptime: "); vga_print_dec(uptime); vga_print(" seconds                                     \n");
			vga_print("                                                                                ");
			vga_print("Press 'q' or Ctrl+X to exit...                                                  ");
		}
		
		__asm__ volatile ("hlt");
	}
	
	process_unregister(pid);
	vga_clear();
}
