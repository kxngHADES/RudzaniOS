[bits 32]
[extern _kernel_main]
[global __start]

section .text

__start:
	; Set up a clean kernel stack
	mov esp, 0x90000

	; Call the C kernel main function
	call _kernel_main

	; If kernel_main returns, halt the CPU
.hang:
	cli
	hlt
	jmp .hang
