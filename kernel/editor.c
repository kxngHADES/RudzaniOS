#include "editor.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "memory.h"
#include "kheap.h"
#include "fs.h"
#include "process.h"

#define MAX_LINES 100
#define LINE_LEN 80

static char lines[MAX_LINES][LINE_LEN];
static int line_lengths[MAX_LINES];
static int num_lines = 1;

static int cx = 0, cy = 0; /* Cursor position within the file */
static int scroll_y = 0;   /* Vertical scroll offset */

static char current_filename[128];
static fs_node_t *cwd;

static void editor_redraw(void) {
	vga_clear();
	
	/* Draw Header */
	vga_set_color(VGA_BLACK, VGA_LIGHT_GREY);
	vga_print(" RudzaniOS Nda - File: ");
	vga_print(current_filename);
	for (int i = 24 + strlen(current_filename); i < VGA_WIDTH; i++) vga_putchar(' ');
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Draw Lines */
	for (int y = 0; y < VGA_HEIGHT - 2; y++) {
		int file_y = y + scroll_y;
		vga_set_cursor(0, y + 1);
		if (file_y < num_lines) {
			for (int x = 0; x < line_lengths[file_y]; x++) {
				vga_putchar(lines[file_y][x]);
			}
		}
	}

	/* Draw Footer */
	vga_set_cursor(0, VGA_HEIGHT - 1);
	vga_set_color(VGA_BLACK, VGA_LIGHT_GREY);
	vga_print(" ^S Save | ^X Exit ");
	for (int i = 19; i < VGA_WIDTH; i++) vga_putchar(' ');
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

	/* Set correct hardware cursor */
	vga_set_cursor(cx, cy - scroll_y + 1);
}

static void editor_save(void) {
	/* Flatten lines into a single buffer */
	uint8_t *buf = (uint8_t*)kmalloc(MAX_LINES * (LINE_LEN + 1));
	uint32_t size = 0;
	
	for (int i = 0; i < num_lines; i++) {
		for (int j = 0; j < line_lengths[i]; j++) {
			buf[size++] = lines[i][j];
		}
		if (i < num_lines - 1) {
			buf[size++] = '\n';
		}
	}

	/* Find or Create file */
	fs_node_t *file = finddir_fs(cwd, current_filename);
	if (!file) {
		create_fs(cwd, current_filename, 0);
		file = finddir_fs(cwd, current_filename);
	}
	
	if (file) {
		write_fs(file, 0, size, buf);
		
		vga_set_cursor(0, VGA_HEIGHT - 1);
		vga_set_color(VGA_BLACK, VGA_LIGHT_GREEN);
		vga_print(" Saved successfully. ");
		vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
		
		/* Pause briefly so user can see it */
		for (volatile int wait = 0; wait < 20000000; wait++);
	}
}

static void editor_load(void) {
	num_lines = 1;
	line_lengths[0] = 0;
	cx = 0;
	cy = 0;
	scroll_y = 0;

	fs_node_t *file = finddir_fs(cwd, current_filename);
	if (!file) return;

	uint8_t *buf = (uint8_t*)kmalloc(file->length);
	read_fs(file, 0, file->length, buf);

	int line = 0;
	int col = 0;
	for (uint32_t i = 0; i < file->length; i++) {
		if (buf[i] == '\n') {
			line_lengths[line] = col;
			line++;
			col = 0;
			if (line >= MAX_LINES) break;
		} else {
			if (col < LINE_LEN - 1) {
				lines[line][col++] = buf[i];
			}
		}
	}
	line_lengths[line] = col;
	num_lines = line + 1;
}

static void handle_input(void) {
	while (1) {
		char c = keyboard_getchar();

		if (c == 24) { /* Ctrl-X */
			vga_clear();
			return;
		} else if (c == 19) { /* Ctrl-S */
			editor_save();
			editor_redraw();
		} else if (c == '\b') { /* Backspace */
			if (cx > 0) {
				cx--;
				for (int i = cx; i < line_lengths[cy] - 1; i++) {
					lines[cy][i] = lines[cy][i + 1];
				}
				line_lengths[cy]--;
			} else if (cy > 0) {
				int old_len = line_lengths[cy - 1];
				/* Append current line to previous line if there's room */
				int copy_len = line_lengths[cy];
				if (old_len + copy_len >= LINE_LEN) copy_len = LINE_LEN - old_len - 1;
				
				for (int i = 0; i < copy_len; i++) {
					lines[cy - 1][old_len + i] = lines[cy][i];
				}
				line_lengths[cy - 1] += copy_len;
				
				/* Shift all lines below up */
				for (int i = cy; i < num_lines - 1; i++) {
					memcpy(lines[i], lines[i + 1], LINE_LEN);
					line_lengths[i] = line_lengths[i + 1];
				}
				num_lines--;
				cy--;
				cx = old_len;
			}
			editor_redraw();
		} else if (c == '\n') {
			if (num_lines < MAX_LINES) {
				/* Shift lines down */
				for (int i = num_lines; i > cy + 1; i--) {
					memcpy(lines[i], lines[i - 1], LINE_LEN);
					line_lengths[i] = line_lengths[i - 1];
				}
				
				/* Move rest of line to new line */
				int chars_to_move = line_lengths[cy] - cx;
				memcpy(lines[cy + 1], &lines[cy][cx], chars_to_move);
				line_lengths[cy + 1] = chars_to_move;
				line_lengths[cy] = cx;
				
				num_lines++;
				cy++;
				cx = 0;
			}
			editor_redraw();
		} else if (c >= 32 && c <= 126) { /* Printable characters */
			if (line_lengths[cy] < LINE_LEN - 1) {
				/* Shift chars right */
				for (int i = line_lengths[cy]; i > cx; i--) {
					lines[cy][i] = lines[cy][i - 1];
				}
				lines[cy][cx] = c;
				line_lengths[cy]++;
				cx++;
			}
			editor_redraw();
		}

		/* Update scrolling if needed */
		if (cy < scroll_y) {
			scroll_y = cy;
			editor_redraw();
		} else if (cy >= scroll_y + VGA_HEIGHT - 2) {
			scroll_y = cy - (VGA_HEIGHT - 3);
			editor_redraw();
		}
	}
}

void editor_start(const char *filename, fs_node_t *dir) {
	uint32_t pid = process_register("editor");
	strcpy(current_filename, filename);
	cwd = dir;
	editor_load();
	editor_redraw();
	handle_input();
	process_unregister(pid);
}
