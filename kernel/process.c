#include "process.h"
#include "vga.h"
#include "string.h"

static process_t processes[MAX_PROCESSES];
static uint32_t next_pid = 1;

void process_init(void) {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		processes[i].active = 0;
	}
	
	/* Register core system processes that are "always running" in the background */
	process_register("kernel_idle");
}

uint32_t process_register(const char *name) {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (!processes[i].active) {
			processes[i].pid = next_pid++;
			strncpy(processes[i].name, name, 31);
			processes[i].name[31] = '\0';
			processes[i].active = 1;
			return processes[i].pid;
		}
	}
	return 0; /* No free slots */
}

void process_unregister(uint32_t pid) {
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (processes[i].active && processes[i].pid == pid) {
			processes[i].active = 0;
			return;
		}
	}
}

void process_list(void) {
	vga_print_colored("PID     Program\n", VGA_LIGHT_CYAN, VGA_BLACK);
	vga_print("------------------------\n");
	
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (processes[i].active) {
			char pid_buf[16];
			itoa(processes[i].pid, pid_buf, 10);
			
			vga_print(pid_buf);
			
			/* Pad with spaces for alignment */
			int pad = 8 - strlen(pid_buf);
			while (pad-- > 0) vga_putchar(' ');
			
			vga_print_line(processes[i].name);
		}
	}
}
