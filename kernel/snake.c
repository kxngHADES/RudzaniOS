#include "games.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "process.h"

#define SNAKE_MAX_LEN 2000
#define WIDTH 80
#define HEIGHT 24

static int snake_x[SNAKE_MAX_LEN];
static int snake_y[SNAKE_MAX_LEN];
static int snake_len = 3;

static int food_x, food_y;
static int score = 0;
static int game_over = 0;
static int dx = 1, dy = 0; /* Moving right by default */

static void spawn_food(void) {
	int valid = 0;
	while (!valid) {
		food_x = (game_rand() % (WIDTH - 2)) + 1;
		food_y = (game_rand() % (HEIGHT - 2)) + 1;
		
		valid = 1;
		for (int i = 0; i < snake_len; i++) {
			if (snake_x[i] == food_x && snake_y[i] == food_y) {
				valid = 0;
				break;
			}
		}
	}
}

static void snake_draw(void) {
	vga_clear();
	
	/* Draw Borders */
	vga_set_color(VGA_LIGHT_BLUE, VGA_BLACK);
	for (int x = 0; x < WIDTH; x++) {
		vga_set_cursor(x, 0); vga_putchar('#');
		vga_set_cursor(x, HEIGHT); vga_putchar('#');
	}
	for (int y = 0; y <= HEIGHT; y++) {
		vga_set_cursor(0, y); vga_putchar('#');
		vga_set_cursor(WIDTH - 1, y); vga_putchar('#');
	}
	
	/* Draw Food */
	vga_set_cursor(food_x, food_y);
	vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
	vga_putchar('@');
	
	/* Draw Snake */
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	for (int i = 0; i < snake_len; i++) {
		vga_set_cursor(snake_x[i], snake_y[i]);
		if (i == 0) vga_putchar('O'); /* Head */
		else vga_putchar('o'); /* Body */
	}
	
	/* Draw Score */
	vga_set_cursor(2, 0);
	vga_set_color(VGA_WHITE, VGA_LIGHT_BLUE);
	vga_print(" Score: ");
	vga_print_dec(score);
	vga_print(" ");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
}

void snake_start(void) {
	uint32_t pid = process_register("snake");
	
	snake_len = 3;
	snake_x[0] = WIDTH / 2; snake_y[0] = HEIGHT / 2;
	snake_x[1] = WIDTH / 2 - 1; snake_y[1] = HEIGHT / 2;
	snake_x[2] = WIDTH / 2 - 2; snake_y[2] = HEIGHT / 2;
	dx = 1; dy = 0;
	score = 0;
	game_over = 0;
	
	spawn_food();
	
	uint32_t last_tick = timer_get_ticks();
	
	while (!game_over) {
		/* Handle Input */
		if (keyboard_has_input()) {
			char c = keyboard_getchar();
			if ((c == 'w' || c == 'W') && dy != 1) { dx = 0; dy = -1; }
			else if ((c == 's' || c == 'S') && dy != -1) { dx = 0; dy = 1; }
			else if ((c == 'a' || c == 'A') && dx != 1) { dx = -1; dy = 0; }
			else if ((c == 'd' || c == 'D') && dx != -1) { dx = 1; dy = 0; }
			else if (c == 'q' || c == 24) game_over = 1; /* Quit */
		}
		
		uint32_t current_tick = timer_get_ticks();
		if (current_tick - last_tick >= 10) { /* Game speed */
			last_tick = current_tick;
			
			/* Move body */
			for (int i = snake_len - 1; i > 0; i--) {
				snake_x[i] = snake_x[i - 1];
				snake_y[i] = snake_y[i - 1];
			}
			/* Move head */
			snake_x[0] += dx;
			snake_y[0] += dy;
			
			/* Collision with walls */
			if (snake_x[0] <= 0 || snake_x[0] >= WIDTH - 1 || 
			    snake_y[0] <= 0 || snake_y[0] >= HEIGHT) {
				game_over = 1;
			}
			
			/* Collision with self */
			for (int i = 1; i < snake_len; i++) {
				if (snake_x[0] == snake_x[i] && snake_y[0] == snake_y[i]) {
					game_over = 1;
				}
			}
			
			/* Eat food */
			if (snake_x[0] == food_x && snake_y[0] == food_y) {
				score += 10;
				if (snake_len < SNAKE_MAX_LEN) snake_len++;
				spawn_food();
			}
			
			if (!game_over) snake_draw();
		}
		
		__asm__ volatile("hlt"); /* Wait for interrupt to save CPU */
	}
	
	/* Game Over Screen */
	vga_set_cursor(WIDTH / 2 - 10, HEIGHT / 2);
	vga_set_color(VGA_WHITE, VGA_LIGHT_RED);
	vga_print(" GAME OVER. Score: ");
	vga_print_dec(score);
	vga_print(" ");
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
	
	/* Wait for keypress to exit */
	while (keyboard_has_input()) keyboard_getchar(); /* clear old input */
	while (!keyboard_has_input()) __asm__ volatile("hlt");
	keyboard_getchar();
	
	process_unregister(pid);
}
