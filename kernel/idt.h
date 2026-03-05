#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* Registers pushed onto the stack by ISR/IRQ stubs */
typedef struct {
	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, esp;
	uint32_t ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

/* Handler function pointer type */
typedef void (*irq_handler_func_t)(registers_t *regs);

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void irq_install_handler(int irq, irq_handler_func_t handler);
void irq_uninstall_handler(int irq);

#endif
