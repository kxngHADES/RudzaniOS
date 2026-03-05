#include "games.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "process.h"

#define T_WIDTH 10
#define T_HEIGHT 20
#define START_X (40 - T_WIDTH/2)
#define START_Y 2

static char board[T_HEIGHT][T_WIDTH];
static int score = 0;
static int game_over = 0;

static int piece_x, piece_y;
static int current_piece;
static int current_rot;

/* 7 Tetrominoes, 4 rotations, 4 blocks each (x, y pairs) */
static const int tetrominos[7][4][4][2] = {
	/* I */
	{ {{0,1},{1,1},{2,1},{3,1}}, {{2,0},{2,1},{2,2},{2,3}}, {{0,2},{1,2},{2,2},{3,2}}, {{1,0},{1,1},{1,2},{1,3}} },
	/* J */
	{ {{0,0},{0,1},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{1,2}}, {{0,1},{1,1},{2,1},{2,2}}, {{1,0},{1,1},{0,2},{1,2}} },
	/* L */
	{ {{2,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{1,2},{2,2}}, {{0,1},{1,1},{2,1},{0,2}}, {{0,0},{1,0},{1,1},{1,2}} },
	/* O */
	{ {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}}, {{1,0},{2,0},{1,1},{2,1}} },
	/* S */
	{ {{1,0},{2,0},{0,1},{1,1}}, {{1,0},{1,1},{2,1},{2,2}}, {{1,1},{2,1},{0,2},{1,2}}, {{0,0},{0,1},{1,1},{1,2}} },
	/* T */
	{ {{1,0},{0,1},{1,1},{2,1}}, {{1,0},{1,1},{2,1},{1,2}}, {{0,1},{1,1},{2,1},{1,2}}, {{1,0},{0,1},{1,1},{1,2}} },
	/* Z */
	{ {{0,0},{1,0},{1,1},{2,1}}, {{2,0},{1,1},{2,1},{1,2}}, {{0,1},{1,1},{1,2},{2,2}}, {{1,0},{0,1},{1,1},{0,2}} }
};

static void spawn_piece(void) {
	current_piece = game_rand() % 7;
	current_rot = 0;
	piece_x = T_WIDTH / 2 - 2;
	piece_y = 0;
	
	/* Game over check */
	for (int i = 0; i < 4; i++) {
		int bx = piece_x + tetrominos[current_piece][current_rot][i][0];
		int by = piece_y + tetrominos[current_piece][current_rot][i][1];
		if (board[by][bx]) {
			game_over = 1;
			break;
		}
	}
}

static int check_collision(int px, int py, int rot) {
	for (int i = 0; i < 4; i++) {
		int bx = px + tetrominos[current_piece][rot][i][0];
		int by = py + tetrominos[current_piece][rot][i][1];
		if (bx < 0 || bx >= T_WIDTH || by >= T_HEIGHT) return 1;
		if (by >= 0 && board[by][bx]) return 1;
	}
	return 0;
}

static void lock_piece(void) {
	for (int i = 0; i < 4; i++) {
		int bx = piece_x + tetrominos[current_piece][current_rot][i][0];
		int by = piece_y + tetrominos[current_piece][current_rot][i][1];
		if (by >= 0) board[by][bx] = 1; /* locked */
	}
	
	/* Check lines */
	int lines_cleared = 0;
	for (int y = T_HEIGHT - 1; y >= 0; y--) {
		int full = 1;
		for (int x = 0; x < T_WIDTH; x++) {
			if (!board[y][x]) { full = 0; break; }
		}
		if (full) {
			lines_cleared++;
			for (int yy = y; yy > 0; yy--) {
				for (int x = 0; x < T_WIDTH; x++) board[yy][x] = board[yy-1][x];
			}
			for (int x = 0; x < T_WIDTH; x++) board[0][x] = 0;
			y++; /* recheck this line */
		}
	}
	score += lines_cleared * 100;
	
	spawn_piece();
}

static void tetris_draw(void) {
	vga_clear();
	
	/* Draw Borders */
	vga_set_color(VGA_LIGHT_BLUE, VGA_BLACK);
	for (int y = 0; y < T_HEIGHT; y++) {
		vga_set_cursor(START_X - 1, START_Y + y); vga_putchar('|');
		vga_set_cursor(START_X + T_WIDTH, START_Y + y); vga_putchar('|');
	}
	for (int x = 0; x <= T_WIDTH + 1; x++) {
		vga_set_cursor(START_X - 1 + x, START_Y + T_HEIGHT); vga_putchar('-');
	}

	/* Draw Score */
	vga_set_cursor(START_X + T_WIDTH + 4, START_Y);
	vga_set_color(VGA_WHITE, VGA_BLACK);
	vga_print("Score: "); vga_print_dec(score);

	/* Draw Board */
	vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
	for (int y = 0; y < T_HEIGHT; y++) {
		for (int x = 0; x < T_WIDTH; x++) {
			vga_set_cursor(START_X + x, START_Y + y);
			if (board[y][x]) vga_putchar('#');
			else vga_putchar('.');
		}
	}

	/* Draw Piece */
	vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
	for (int i = 0; i < 4; i++) {
		int bx = piece_x + tetrominos[current_piece][current_rot][i][0];
		int by = piece_y + tetrominos[current_piece][current_rot][i][1];
		if (by >= 0) {
			vga_set_cursor(START_X + bx, START_Y + by);
			vga_putchar('#');
		}
	}
}

void tetris_start(void) {
	uint32_t pid = process_register("tetris");
	
	for (int y = 0; y < T_HEIGHT; y++)
		for (int x = 0; x < T_WIDTH; x++)
			board[y][x] = 0;
			
	score = 0;
	game_over = 0;
	spawn_piece();
	
	uint32_t last_drop = timer_get_ticks();
	
	while (!game_over) {
		if (keyboard_has_input()) {
			char c = keyboard_getchar();
			if ((c == 'a' || c == 'A') && !check_collision(piece_x - 1, piece_y, current_rot)) piece_x--;
			else if ((c == 'd' || c == 'D') && !check_collision(piece_x + 1, piece_y, current_rot)) piece_x++;
			else if ((c == 's' || c == 'S') && !check_collision(piece_x, piece_y + 1, current_rot)) piece_y++;
			else if (c == 'w' || c == 'W') { /* Rotate */
				int new_rot = (current_rot + 1) % 4;
				if (!check_collision(piece_x, piece_y, new_rot)) current_rot = new_rot;
			}
			else if (c == 'q' || c == 24) game_over = 1;
			
			if (!game_over) tetris_draw();
		}
		
		uint32_t current_tick = timer_get_ticks();
		if (current_tick - last_drop >= 50) { /* Game Speed */
			last_drop = current_tick;
			if (!check_collision(piece_x, piece_y + 1, current_rot)) {
				piece_y++;
			} else {
				lock_piece();
			}
			if (!game_over) tetris_draw();
		}
		
		__asm__ volatile("hlt");
	}
	
	process_unregister(pid);
}
