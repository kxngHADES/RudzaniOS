#include "keyboard.h"
#include "idt.h"
#include "port.h"
#include "vga.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define BUFFER_SIZE 256

/* Circular keyboard input buffer */
static char key_buffer[BUFFER_SIZE];
static volatile int buf_head = 0;
static volatile int buf_tail = 0;

/* Key state flags */
static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int caps_lock = 0;

/* Scancode set 1 to ASCII (US QWERTY, unshifted) */
static const char scancode_ascii[128] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
	'*', 0, ' ', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* F1-F10 */
	0, 0, /* Num Lock, Scroll Lock */
	0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, /* Keypad */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Scancode set 1 to ASCII (US QWERTY, shifted) */
static const char scancode_ascii_shift[128] = {
	0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
	0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
	'*', 0, ' ', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0,
	0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static void buffer_put(char c) {
	int next = (buf_head + 1) % BUFFER_SIZE;
	if (next != buf_tail) {
		key_buffer[buf_head] = c;
		buf_head = next;
	}
}

static void keyboard_callback(registers_t *regs) {
	/* Read status register to check if data is available */
	uint8_t status = port_byte_in(KEYBOARD_STATUS_PORT);
	if (!(status & 0x01)) return;

	uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);
	(void)regs;

	/* Key release (bit 7 set) */
	if (scancode & 0x80) {
		uint8_t released = scancode & 0x7F;
		if (released == 0x2A || released == 0x36) {
			shift_pressed = 0;
		} else if (released == 0x1D) {
			ctrl_pressed = 0;
		}
		return;
	}

	/* Handle special keys */
	switch (scancode) {
		case 0x2A: /* Left Shift */
		case 0x36: /* Right Shift */
			shift_pressed = 1;
			return;
		case 0x1D: /* Left Ctrl */
			ctrl_pressed = 1;
			return;
		case 0x3A: /* Caps Lock */
			caps_lock = !caps_lock;
			return;
	}

	/* Translate and buffer */
	char c;
	int use_shift = shift_pressed ^ caps_lock;
	if (scancode < 128) {
		c = use_shift ? scancode_ascii_shift[scancode] : scancode_ascii[scancode];
		if (ctrl_pressed && c >= 'a' && c <= 'z') {
			c = c - 'a' + 1; /* Translate to ASCII 1-26 */
		}
		if (c) buffer_put(c);
	}
}

void keyboard_init(void) {
	/* Clear any pending data in the PS/2 buffer */
	while (port_byte_in(KEYBOARD_STATUS_PORT) & 0x01) {
		port_byte_in(KEYBOARD_DATA_PORT);
	}

	irq_install_handler(1, keyboard_callback);
}

int keyboard_has_input(void) {
	return buf_head != buf_tail;
}

char keyboard_getchar(void) {
	/* Busy-wait until a character is available */
	while (buf_head == buf_tail) {
		__asm__ volatile("hlt");
	}
	char c = key_buffer[buf_tail];
	buf_tail = (buf_tail + 1) % BUFFER_SIZE;
	return c;
}
