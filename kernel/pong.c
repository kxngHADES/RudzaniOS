#include "games.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "process.h"

#define P_WIDTH 80
#define P_HEIGHT 24
#define PADDLE_HEIGHT 5

static int ball_x, ball_y;
static int ball_dx, ball_dy;
static int paddle_y; /* player paddle (right) */
static int ai_paddle_y; /* AI paddle (left) */
static int p_score = 0;
static int ai_score = 0;
static int p_game_over = 0;

static void pong_draw(void) {
	vga_clear();

	/* Draw borders */
	vga_set_color(VGA_LIGHT_BLUE, VGA_BLACK);
	for (int x = 0; x < P_WIDTH; x++) {
		vga_set_cursor(x, 0); vga_putchar('-');
		vga_set_cursor(x, P_HEIGHT); vga_putchar('-');
	}

	/* Draw net */
	for (int y = 1; y < P_HEIGHT; y += 2) {
		vga_set_cursor(P_WIDTH / 2, y); vga_putchar('|');
	}

	/* Draw scores */
	vga_set_cursor(P_WIDTH / 4, 1);
	vga_set_color(VGA_WHITE, VGA_BLACK);
	vga_print_dec(ai_score);
	vga_set_cursor(3 * P_WIDTH / 4, 1);
	vga_print_dec(p_score);

	/* Draw Paddles */
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	for (int i = 0; i < PADDLE_HEIGHT; i++) {
		vga_set_cursor(1, ai_paddle_y + i); vga_putchar('|');
		vga_set_cursor(P_WIDTH - 2, paddle_y + i); vga_putchar('|');
	}

	/* Draw Ball */
	vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
	vga_set_cursor(ball_x, ball_y);
	vga_putchar('O');
}

void pong_start(void) {
	uint32_t pid = process_register("pong");
	
	ball_x = P_WIDTH / 2;
	ball_y = P_HEIGHT / 2;
	ball_dx = 1; ball_dy = 1;
	paddle_y = P_HEIGHT / 2 - PADDLE_HEIGHT / 2;
	ai_paddle_y = P_HEIGHT / 2 - PADDLE_HEIGHT / 2;
	p_score = 0; ai_score = 0;
	p_game_over = 0;

	uint32_t last_tick = timer_get_ticks();
	uint32_t move_tick = 0;
	
	while (!p_game_over) {
		if (keyboard_has_input()) {
			char c = keyboard_getchar();
			if ((c == 'w' || c == 'W') && paddle_y > 1) paddle_y -= 2; /* Move faster */
			else if ((c == 's' || c == 'S') && paddle_y < P_HEIGHT - PADDLE_HEIGHT) paddle_y += 2;
			else if (c == 'q' || c == 24) p_game_over = 1;
		}

		uint32_t current_tick = timer_get_ticks();
		if (current_tick - last_tick >= 5) { /* Ball speed */
			last_tick = current_tick;
			move_tick++;

			ball_x += ball_dx;
			ball_y += ball_dy;

			/* Top/Bottom collision */
			if (ball_y <= 1 || ball_y >= P_HEIGHT - 1) {
				ball_dy = -ball_dy;
			}

			/* Left Paddle (AI) collision */
			if (ball_x == 2 && ball_y >= ai_paddle_y && ball_y < ai_paddle_y + PADDLE_HEIGHT) {
				ball_dx = -ball_dx;
				ball_x = 3; /* prevent sticking */
				/* Simple angle adjustment */
				if (ball_y == ai_paddle_y) ball_dy = -1;
				else if (ball_y == ai_paddle_y + PADDLE_HEIGHT - 1) ball_dy = 1;
			}

			/* Right Paddle (Player) collision */
			if (ball_x == P_WIDTH - 3 && ball_y >= paddle_y && ball_y < paddle_y + PADDLE_HEIGHT) {
				ball_dx = -ball_dx;
				ball_x = P_WIDTH - 4;
				if (ball_y == paddle_y) ball_dy = -1;
				else if (ball_y == paddle_y + PADDLE_HEIGHT - 1) ball_dy = 1;
			}

			/* Scoring */
			if (ball_x <= 0) {
				p_score++;
				ball_x = P_WIDTH / 2; ball_y = P_HEIGHT / 2;
				ball_dx = 1; /* Serve to player */
			} else if (ball_x >= P_WIDTH - 1) {
				ai_score++;
				ball_x = P_WIDTH / 2; ball_y = P_HEIGHT / 2;
				ball_dx = -1; /* Serve to AI */
			}

			/* AI Logic (moves every other frame to simulate reaction time) */
			if (move_tick % 2 == 0) {
				int center = ai_paddle_y + PADDLE_HEIGHT / 2;
				if (center < ball_y && ai_paddle_y < P_HEIGHT - PADDLE_HEIGHT) ai_paddle_y++;
				else if (center > ball_y && ai_paddle_y > 1) ai_paddle_y--;
			}

			pong_draw();
		}
		
		__asm__ volatile("hlt");
	}

	process_unregister(pid);
}
