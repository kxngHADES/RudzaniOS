#include "timer.h"
#include "idt.h"
#include "port.h"

#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_FREQUENCY 1193182

static volatile uint32_t tick_count = 0;

static void timer_callback(registers_t *regs) {
	(void)regs;
	tick_count++;
}

void timer_init(uint32_t frequency) {
	uint32_t divisor = PIT_FREQUENCY / frequency;

	/* Send command byte: Channel 0, lobyte/hibyte, rate generator */
	port_byte_out(PIT_COMMAND, 0x36);

	/* Send divisor (low byte first, then high byte) */
	port_byte_out(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
	port_byte_out(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

	irq_install_handler(0, timer_callback);
}

uint32_t timer_get_ticks(void) {
	return tick_count;
}

void timer_wait(uint32_t ticks) {
	uint32_t target = tick_count + ticks;
	while (tick_count < target) {
		__asm__ volatile("hlt");
	}
}
