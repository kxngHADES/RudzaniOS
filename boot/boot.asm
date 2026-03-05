org 0x7c00
bits 16

start:
	; Setup data segments and stack
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7c00

	; Save the boot drive passed by BIOS in DL
	mov [BOOT_DRIVE], dl

	mov si, message
	call print_string

load_kernel:
	; Check for LBA support
	mov ah, 0x41
	mov bx, 0x55AA
	mov dl, [BOOT_DRIVE]
	int 0x13
	jc disk_error

	; Read kernel using Extended Read (LBA)
	mov ah, 0x42
	mov dl, [BOOT_DRIVE]
	mov si, dap
	int 0x13
	jc disk_error

	; Enable A20 line (fast method via port 0x92)
	in al, 0x92
	or al, 0x02
	out 0x92, al

	; Load the Global Descriptor Table
	lgdt [gdt_descriptor]

	; Switch to 32-bit protected mode
	cli
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax

	; Far jump to flush prefetch queue and enter 32-bit code
	; Segment 0x08 is the code descriptor in our GDT
	jmp 0x08:protected_mode

disk_error:
	mov si, error_msg
	call print_string
	jmp done

print_string:
.loop:
	lodsb
	cmp al, 0
	je .done
	mov ah, 0x0E
	int 0x10
	jmp .loop
.done:
	ret

done:
	jmp $

; ============================================
; GDT (Global Descriptor Table)
; ============================================

gdt_start:
	; Null descriptor (required)
	dd 0x00000000
	dd 0x00000000

gdt_code:
	; Code segment: Base=0, Limit=4GB, Executable, Readable, Ring 0
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 10011010b
	db 11001111b
	db 0x00

gdt_data:
	; Data segment: Base=0, Limit=4GB, Writable, Ring 0
	dw 0xFFFF
	dw 0x0000
	db 0x00
	db 10010010b
	db 11001111b
	db 0x00

gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

; ============================================
; 32-bit Protected Mode Entry
; ============================================
bits 32

protected_mode:
	; Set all data segment registers to the kernel data selector (0x10)
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	; Set up kernel stack at 0x90000
	mov esp, 0x90000

	; Jump to the kernel entry point loaded at 0x10000
	jmp 0x10000

; ============================================
; Data
; ============================================

BOOT_DRIVE db 0
message db "RudzaniOS Bootloader v1.0", 13, 10, 0
error_msg db "Disk read error!", 13, 10, 0

align 4
dap:
	db 0x10		; size of DAP
	db 0		; unused
	dw 120		; number of sectors to read
	dw 0x0000	; offset
	dw 0x1000	; segment
	dq 1		; start sector (sector 1 = kernel)

times 510-($-$$) db 0
dw 0xAA55