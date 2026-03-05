#ifndef VGA_H
#define VGA_H

#include <stdint.h>

/* VGA text mode color constants */
#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_BROWN         0x6
#define VGA_LIGHT_GREY    0x7
#define VGA_DARK_GREY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xA
#define VGA_LIGHT_CYAN    0xB
#define VGA_LIGHT_RED     0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW        0xE
#define VGA_WHITE         0xF

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

void vga_init(void);
void vga_clear(void);
void vga_putchar(char c);
void vga_print(const char *str);
void vga_print_line(const char *str);
void vga_print_colored(const char *str, uint8_t fg, uint8_t bg);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_set_cursor(int x, int y);
void vga_get_cursor(int *x, int *y);
void vga_print_hex(uint32_t value);
void vga_print_dec(int value);

#endif
