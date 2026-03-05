ENTRY(__start)

SECTIONS
{
	/* Relocated from 0x1000 to 0x10000 to avoid bootloader collision */
	. = 0x10000;

	.text : {
		*(.text)
	}

	.rodata : {
		*(.rodata*)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
		*(COMMON)
	}
}
