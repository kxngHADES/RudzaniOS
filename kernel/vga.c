#include "vga.h"
#include "port.h"
#include "string.h"

#define VGA_MEMORY 0xB8000
#define VGA_CTRL_PORT 0x3D4
#define VGA_DATA_PORT 0x3D5

static uint16_t *vga_buffer = (uint16_t *)VGA_MEMORY;
static uint8_t vga_color;
static int cursor_x = 0;
static int cursor_y = 0;

static uint16_t vga_entry(char c, uint8_t color) {
	return (uint16_t)c | ((uint16_t)color << 8);
}

static uint8_t vga_make_color(uint8_t fg, uint8_t bg) {
	return fg | (bg << 4);
}

static void vga_update_cursor(void) {
	uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
	port_byte_out(VGA_CTRL_PORT, 14);
	port_byte_out(VGA_DATA_PORT, (uint8_t)(pos >> 8));
	port_byte_out(VGA_CTRL_PORT, 15);
	port_byte_out(VGA_DATA_PORT, (uint8_t)(pos & 0xFF));
}

static void vga_scroll(void) {
	if (cursor_y >= VGA_HEIGHT) {
		int i;
		/* Move all lines up by one */
		for (i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
			vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
		}
		/* Clear the last line */
		for (i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++) {
			vga_buffer[i] = vga_entry(' ', vga_color);
		}
		cursor_y = VGA_HEIGHT - 1;
	}
}

void vga_init(void) {
	vga_color = vga_make_color(VGA_LIGHT_GREY, VGA_BLACK);
	vga_clear();
}

void vga_clear(void) {
	int i;
	for (i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
		vga_buffer[i] = vga_entry(' ', vga_color);
	}
	cursor_x = 0;
	cursor_y = 0;
	vga_update_cursor();
}

void vga_putchar(char c) {
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else if (c == '\r') {
		cursor_x = 0;
	} else if (c == '\t') {
		cursor_x = (cursor_x + 4) & ~3;
	} else if (c == '\b') {
		if (cursor_x > 0) {
			cursor_x--;
			vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(' ', vga_color);
		}
	} else {
		vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = vga_entry(c, vga_color);
		cursor_x++;
	}

	if (cursor_x >= VGA_WIDTH) {
		cursor_x = 0;
		cursor_y++;
	}

	vga_scroll();
	vga_update_cursor();
}

void vga_print(const char *str) {
	while (*str) {
		vga_putchar(*str++);
	}
}

void vga_print_line(const char *str) {
	vga_print(str);
	vga_putchar('\n');
}

void vga_print_colored(const char *str, uint8_t fg, uint8_t bg) {
	uint8_t old_color = vga_color;
	vga_color = vga_make_color(fg, bg);
	vga_print(str);
	vga_color = old_color;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
	vga_color = vga_make_color(fg, bg);
}

void vga_set_cursor(int x, int y) {
	cursor_x = x;
	cursor_y = y;
	vga_update_cursor();
}

void vga_get_cursor(int *x, int *y) {
	*x = cursor_x;
	*y = cursor_y;
}


void vga_print_hex(uint32_t value) {
	char buf[11];
	char *p = buf;
	int i;
	int started = 0;

	*p++ = '0';
	*p++ = 'x';

	for (i = 28; i >= 0; i -= 4) {
		uint8_t nibble = (value >> i) & 0xF;
		if (nibble || started || i == 0) {
			started = 1;
			*p++ = (nibble < 10) ? '0' + nibble : 'A' + nibble - 10;
		}
	}
	*p = '\0';
	vga_print(buf);
}

void vga_print_dec(int value) {
	char buf[12];
	itoa(value, buf, 10);
	vga_print(buf);
}
