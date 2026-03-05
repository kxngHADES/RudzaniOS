[bits 32]
[extern _isr_handler]
[extern _irq_handler]

section .text

; ============================================
; ISR Macros
; ============================================

; ISR with no error code (push dummy 0)
%macro ISR_NOERRCODE 1
[global _isr%1]
_isr%1:
	push dword 0
	push dword %1
	jmp isr_common_stub
%endmacro

; ISR with error code (CPU already pushed it)
%macro ISR_ERRCODE 1
[global _isr%1]
_isr%1:
	push dword %1
	jmp isr_common_stub
%endmacro

; IRQ stub (push dummy error code + remapped interrupt number)
%macro IRQ 2
[global _irq%1]
_irq%1:
	push dword 0
	push dword %2
	jmp irq_common_stub
%endmacro

; ============================================
; CPU Exception ISRs (0-31)
; ============================================

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; ============================================
; Hardware IRQs (0-15 remapped to INT 32-47)
; ============================================

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; ============================================
; Common ISR Stub (CPU Exceptions)
; ============================================

isr_common_stub:
	pushad
	push ds
	push es
	push fs
	push gs

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push esp
	call _isr_handler
	add esp, 4

	pop gs
	pop fs
	pop es
	pop ds
	popad
	add esp, 8
	iret

; ============================================
; Common IRQ Stub (Hardware Interrupts)
; ============================================

irq_common_stub:
	pushad
	push ds
	push es
	push fs
	push gs

	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push esp
	call _irq_handler
	add esp, 4

	pop gs
	pop fs
	pop es
	pop ds
	popad
	add esp, 8
	iret
