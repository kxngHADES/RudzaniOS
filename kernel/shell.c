#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "timer.h"
#include "memory.h"
#include "kheap.h"
#include "fs.h"
#include "port.h"
#include "editor.h"
#include "sysmon.h"
#include "calc.h"
#include "process.h"
#include "games.h"

#define INPUT_BUFFER_SIZE 256

static char input_buffer[INPUT_BUFFER_SIZE];
static int buffer_idx = 0;

/* Current working directory */
static fs_node_t *current_dir = 0;
static char current_path[256] = "/";

static void execute_command(char *line);

static void print_prompt(void) {
	vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
	vga_print("RudzaniOS:");
	vga_print(current_path);
	vga_print("> ");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
}

static void cmd_help(void) {
	vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
	vga_print_line("Available Commands:");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
	vga_print_line("  help      - Show this help message");
	vga_print_line("  clear     - Clear the screen");
	vga_print_line("  about     - Display OS information");
	vga_print_line("  uptime    - Show ticks since boot");
	vga_print_line("  meminfo   - Show memory usage");
	vga_print_line("  malloc    - Test heap allocation");
	vga_print_line("  ls        - List files");
	vga_print_line("  show_me <f> - Print file contents");
	vga_print_line("  cd <dir>  - Change directory");
	vga_print_line("  pwd       - Print working directory");
	vga_print_line("  mkdir <d> - Create directory");
	vga_print_line("  touch <f> - Create empty file");
	vga_print_line("  rm <f/d>  - Remove file or directory");
	vga_print_line("  nda <f>   - Text editor");
	vga_print_line("  sysmon    - Dynamic system monitor (top)");
	vga_print_line("  calc <eq> - Mathematical calculator");
	vga_print_line("  ps        - List running processes");
	vga_print_line("  lets_play - Play Terminal Games");
	vga_print_line("  cp <srd> <dst> - Copy file");
	vga_print_line("  cut <src> <dst> - Move (cut) file");
	vga_print_line("  echo      - Echo text back");
	vga_print_line("  reboot    - Reboot the machine");
	vga_print_line("  halt      - Halt the CPU");
}

static void cmd_about(void) {
	vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
	vga_print_line("================================");
	vga_print_line("  RudzaniOS v0.1.0");
	vga_print_line("  A minimal x86 kernel");
	vga_print_line("  Written in C and Assembly");
	vga_print_line("================================");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
	vga_print_line("");
	vga_print_line("Features:");
	vga_print_line("  - 32-bit protected mode");
	vga_print_line("  - VGA text-mode display");
	vga_print_line("  - IDT with ISR/IRQ handling");
	vga_print_line("  - PS/2 keyboard driver");
	vga_print_line("  - PIT timer");
	vga_print_line("  - Physical memory manager");
	vga_print_line("  - Interactive shell");
}

static void cmd_uptime(void) {
	uint32_t ticks = timer_get_ticks();
	uint32_t seconds = ticks / 100;
	uint32_t minutes = seconds / 60;

	vga_print("Uptime: ");
	vga_print_dec(ticks);
	vga_print(" ticks (");
	vga_print_dec(minutes);
	vga_print("m ");
	vga_print_dec(seconds % 60);
	vga_print("s)\n");
}

static void cmd_meminfo(void) {
	vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
	vga_print_line("Memory Information:");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	vga_print("  Page size:    4 KB\n");
	vga_print("  Total pages:  "); vga_print_dec(mem_get_total_pages());
	vga_print(" ("); vga_print_dec(mem_get_total_pages() * 4); vga_print(" KB)\n");
	vga_print("  Used pages:   "); vga_print_dec(mem_get_used_pages());
	vga_print(" ("); vga_print_dec(mem_get_used_pages() * 4); vga_print(" KB)\n");
	vga_print("  Free pages:   "); vga_print_dec(mem_get_free_pages());
	vga_print(" ("); vga_print_dec(mem_get_free_pages() * 4); vga_print(" KB)\n");
}

static void cmd_ls(const char *args) {
	(void)args;
	int i = 0;
	struct dirent *node = 0;
	while ((node = readdir_fs(current_dir, i)) != 0) {
		vga_print("  ");
		vga_print_line(node->name);
		i++;
	}
}

static void cmd_show_me(const char *args) {
	if (!args || !*args) {
		vga_print_line("Usage: show_me <filename>");
		return;
	}

	fs_node_t *node = finddir_fs(current_dir, (char*)args);
	if (node) {
		uint8_t buf[1024];
		uint32_t sz = read_fs(node, 0, 1024, buf);
		buf[sz] = 0;
		vga_print_line((char*)buf);
	} else {
		vga_print("File not found: ");
		vga_print_line(args);
	}
}

static void cmd_pwd(const char *args) {
	(void)args;
	vga_print_line(current_path);
}

static fs_node_t *path_to_node(const char *path) {
	if (!path || !*path) return 0;
	
	fs_node_t *start = current_dir;
	if (path[0] == '/') {
		start = fs_root;
		path++;
		if (!*path) return fs_root;
	}

	char temp[256];
	strcpy(temp, path);
	char *part = temp;
	char *next = 0;
	fs_node_t *curr = start;

	while (part && *part) {
		next = 0;
		for (int i = 0; part[i]; i++) {
			if (part[i] == '/') {
				part[i] = '\0';
				next = &part[i+1];
				break;
			}
		}

		if (strcmp(part, "..") == 0) {
			if (curr->parent) curr = curr->parent;
		} else if (strcmp(part, ".") != 0) {
			fs_node_t *node = finddir_fs(curr, part);
			if (!node) return 0;
			curr = node;
		}
		part = next;
	}
	return curr;
}

static void cmd_cd(const char *args) {
	if (!args || !*args) {
		current_dir = fs_root;
		strcpy(current_path, "/");
		return;
	}

	fs_node_t *node = path_to_node(args);
	if (node && (node->flags & FS_DIRECTORY)) {
		current_dir = node;
		
		/* Rebuild absolute path string */
		if (args[0] == '/') {
			strcpy(current_path, "/");
			if (args[1]) {
				if (args[strlen(args)-1] == '/') {
					strncat(current_path, args + 1, strlen(args)-2);
				} else {
					strcat(current_path, args + 1);
				}
			}
		} else {
			/* Simplistic relative update - better would be full traversal from root */
			if (strcmp(args, "..") == 0) {
				char *last_slash = 0;
				for (int i = 0; current_path[i]; i++) {
					if (current_path[i] == '/') last_slash = &current_path[i];
				}
				if (last_slash == current_path) current_path[1] = '\0';
				else if (last_slash) *last_slash = '\0';
			} else {
				if (strcmp(current_path, "/") != 0) strcat(current_path, "/");
				strcat(current_path, args);
			}
		}
	} else {
		vga_print("Directory not found: ");
		vga_print_line(args);
	}
}

static void cmd_mkdir(const char *args) {
	if (!args || !*args) {
		vga_print_line("Usage: mkdir <dirname>");
		return;
	}
	mkdir_fs(current_dir, (char*)args, 0);
	vga_print("Directory created: ");
	vga_print_line(args);
}

static void cmd_touch(const char *args) {
	if (!args || !*args) {
		vga_print_line("Usage: touch <filename>");
		return;
	}
	create_fs(current_dir, (char*)args, 0);
	vga_print("File created: ");
	vga_print_line(args);
}

static void cmd_cp(const char *args) {
	char src[64], dst[64];
	int i = 0, j = 0;
	while (args[i] && args[i] != ' ') { src[j++] = args[i++]; }
	src[j] = '\0';
	while (args[i] == ' ') i++;
	j = 0;
	while (args[i] && args[i] != ' ') { dst[j++] = args[i++]; }
	dst[j] = '\0';

	if (src[0] == '\0' || dst[0] == '\0') {
		vga_print_line("Usage: cp <source> <destination>");
		return;
	}

	fs_node_t *src_node = finddir_fs(current_dir, src);
	if (!src_node) {
		vga_print("Source not found: ");
		vga_print_line(src);
		return;
	}

	create_fs(current_dir, dst, 0);
	fs_node_t *dst_node = finddir_fs(current_dir, dst);
	
	uint8_t buf[1024];
	uint32_t sz = read_fs(src_node, 0, 1024, buf);
	write_fs(dst_node, 0, sz, buf);

	vga_print("Copied "); vga_print(src); vga_print(" to "); vga_print_line(dst);
}

static void run_script(fs_node_t *file) {
	uint32_t size = file->length;
	if (size == 0) return;

	uint8_t *buf = (uint8_t*)kmalloc(size + 1);
	read_fs(file, 0, size, buf);
	buf[size] = '\0';

	char line[INPUT_BUFFER_SIZE];
	uint32_t p = 0;
	
	while (p < size) {
		int i = 0;
		while (p < size && buf[p] != '\n' && i < INPUT_BUFFER_SIZE - 1) {
			line[i++] = buf[p++];
		}
		line[i] = '\0';
		if (buf[p] == '\n') p++;

		/* Execute if not empty and not a comment */
		if (line[0] != '\0' && line[0] != '#') {
			execute_command(line);
		}
	}
	kfree(buf);
}

static void cmd_cut(const char *args) {
	char src[64], dst[64];
	int i = 0, j = 0;
	while (args[i] && args[i] != ' ') { src[j++] = args[i++]; }
	src[j] = '\0';
	while (args[i] == ' ') i++;
	j = 0;
	while (args[i] && args[i] != ' ') { dst[j++] = args[i++]; }
	dst[j] = '\0';

	if (src[0] == '\0' || dst[0] == '\0') {
		vga_print_line("Usage: cut <source> <destination>");
		return;
	}

	cmd_cp(args);
	unlink_fs(current_dir, src);
	vga_print("Moved (cut) "); vga_print(src); vga_print(" to "); vga_print_line(dst);
}

static void cmd_rm(const char *args) {
	if (!args || !*args) {
		vga_print_line("Usage: rm <filename/dirname>");
		return;
	}
	unlink_fs(current_dir, (char*)args);
	vga_print("Removed: ");
	vga_print_line(args);
}

static void cmd_nda(const char *args) {
	if (!args || !*args) {
		vga_print_line("Usage: nda <filename>");
		return;
	}
	editor_start(args, current_dir);
}

static void cmd_echo(const char *args) {
	if (!args || !*args) {
		vga_print_line("");
		return;
	}

	/* Check if it's a file first */
	fs_node_t *node = finddir_fs(current_dir, (char*)args);
	if (node && !(node->flags & FS_DIRECTORY)) {
		uint8_t buf[1024];
		uint32_t sz = read_fs(node, 0, 1024, buf);
		buf[sz] = 0;
		vga_print_line((char*)buf);
	} else {
		vga_print_line(args);
	}
}

static void cmd_reboot(void) {
	vga_print_line("Rebooting...");
	uint8_t good = 0x02;
	while (good & 0x02) good = port_byte_in(0x64);
	port_byte_out(0x64, 0xFE);
	__asm__ volatile("cli; hlt");
}

static void cmd_halt(void) {
	vga_set_color(VGA_YELLOW, VGA_BLACK);
	vga_print_line("System halted. You can safely power off.");
	__asm__ volatile("cli; hlt");
}

static void execute_command(char *line) {
	if (!line || line[0] == '\0') return;

	char *cmd = line;
	char *args = line;
	while (*args && *args != ' ') args++;
	if (*args == ' ') {
		*args = '\0';
		args++;
		while (*args == ' ') args++;
	}

	if (strcmp(cmd, "help") == 0) cmd_help();
	else if (strcmp(cmd, "clear") == 0) vga_clear();
	else if (strcmp(cmd, "about") == 0) cmd_about();
	else if (strcmp(cmd, "uptime") == 0) cmd_uptime();
	else if (strcmp(cmd, "meminfo") == 0) cmd_meminfo();
	else if (strcmp(cmd, "ls") == 0) cmd_ls(args);
	else if (strcmp(cmd, "show_me") == 0) cmd_show_me(args);
	else if (strcmp(cmd, "cd") == 0) cmd_cd(args);
	else if (strcmp(cmd, "pwd") == 0) cmd_pwd(args);
	else if (strcmp(cmd, "mkdir") == 0) cmd_mkdir(args);
	else if (strcmp(cmd, "touch") == 0) cmd_touch(args);
	else if (strcmp(cmd, "rm") == 0) cmd_rm(args);
	else if (strcmp(cmd, "nda") == 0) cmd_nda(args);
	else if (strcmp(cmd, "sysmon") == 0 || strcmp(cmd, "top") == 0) sysmon_start();
	else if (strcmp(cmd, "calc") == 0) cmd_calc(args);
	else if (strcmp(cmd, "ps") == 0) process_list();
	else if (strcmp(cmd, "lets_play") == 0) cmd_lets_play();
	else if (strcmp(cmd, "cp") == 0) cmd_cp(args);
	else if (strcmp(cmd, "cut") == 0) cmd_cut(args);
	else if (strcmp(cmd, "echo") == 0) cmd_echo(args);
	else if (strcmp(cmd, "reboot") == 0) cmd_reboot();
	else if (strcmp(cmd, "halt") == 0) cmd_halt();
	else {
		/* Check if it's a script file in current dir */
		fs_node_t *node = finddir_fs(current_dir, cmd);
		if (!node && bin_dir) {
			/* SEARCH PATH: Check in /bin */
			node = finddir_fs(bin_dir, cmd);
		}

		if (node) {
			int len = strlen(cmd);
			if (len > 3 && strcmp(&cmd[len - 3], ".sh") == 0) {
				run_script(node);
				return;
			}
		}

		vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
		vga_print("Unknown command: "); vga_print_line(cmd);
		vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
	}
}

static void shell_tab_complete(void) {
	if (buffer_idx == 0) return;

	/* Find start of last word */
	int word_start = buffer_idx;
	while (word_start > 0 && input_buffer[word_start - 1] != ' ') {
		word_start--;
	}

	char prefix[64];
	int len = buffer_idx - word_start;
	if (len >= 63) return;
	memcpy(prefix, &input_buffer[word_start], len);
	prefix[len] = '\0';

	char match_name[128];
	int match_count = 0;
	int i = 0;
	struct dirent *node = 0;
	
	while ((node = readdir_fs(current_dir, i)) != 0) {
		if (strncmp(node->name, prefix, len) == 0) {
			strcpy(match_name, node->name);
			match_count++;
		}
		i++;
	}

	if (match_count == 1) {
		/* Complete the word */
		char *to_add = match_name + len;
		while (*to_add) {
			if (buffer_idx < INPUT_BUFFER_SIZE - 1) {
				input_buffer[buffer_idx++] = *to_add;
				vga_putchar(*to_add);
			}
			to_add++;
		}
	}
}

void shell_run(void) {
	current_dir = fs_root;
	
	while (1) {
		print_prompt();
		buffer_idx = 0;
		memset(input_buffer, 0, INPUT_BUFFER_SIZE);

		while (1) {
			char c = keyboard_getchar();
			if (c == '\n') {
				vga_putchar('\n');
				input_buffer[buffer_idx] = '\0';
				break;
			} else if (c == '\b') {
				if (buffer_idx > 0) {
					buffer_idx--;
					vga_putchar('\b');
				}
			} else if (c == '\t') {
				shell_tab_complete();
			} else if (c >= 32 && c <= 126) {
				if (buffer_idx < INPUT_BUFFER_SIZE - 1) {
					input_buffer[buffer_idx++] = c;
					vga_putchar(c);
				}
			}
		}

		if (buffer_idx > 0) {
			execute_command(input_buffer);
		}
	}
}
