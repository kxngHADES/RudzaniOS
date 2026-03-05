#include "games.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"

static unsigned int rand_seed = 1;

int game_rand(void) {
	rand_seed = rand_seed * 1103515245 + 12345;
	return (unsigned int)(rand_seed / 65536) % 32768;
}

void cmd_lets_play(void) {
	vga_clear();
	while (1) {
		vga_set_cursor(0, 5);
		vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
		
		vga_print_line("      ============================================");
		vga_print_line("               RudzaniOS Arcade Machine           ");
		vga_print_line("      ============================================");
		vga_print_line("");
		vga_print_line("      Please select a game to play:");
		vga_print_line("");
		vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
		vga_print_line("        [1] Snake  (Controls: W, A, S, D)");
		vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
		vga_print_line("        [2] Pong   (Controls: W, S)");
		vga_set_color(VGA_YELLOW, VGA_BLACK);
		vga_print_line("        [3] Tetris (Controls: A, D, W, S)");
		vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
		vga_print_line("");
		vga_print_line("        [q] Quit to Shell (All games use 'q' to exit)");
		
		while (!keyboard_has_input()) {
			__asm__ volatile("hlt");
		}
		
		char c = keyboard_getchar();
		rand_seed += timer_get_ticks(); /* Seed the PRNG based on user input time */
		
		if (c == '1') {
			snake_start();
			vga_clear();
		} else if (c == '2') {
			pong_start();
			vga_clear();
		} else if (c == '3') {
			tetris_start();
			vga_clear();
		} else if (c == 'q' || c == 'Q') {
			break;
		}
	}
	vga_clear();
}
