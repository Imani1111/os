OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(launch_kernel)

SECTIONS
{
	. = 0x100000;
	
	.text : {
		*(.text.prologue)
		*(.text)
	}  
	
	. = ALIGN(4096);	

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

	_kernel_end = .;
}
