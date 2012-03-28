OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_start)
STARTUP(inittask2.o)
SEARCH_DIR(.)
SECTIONS
{
	.text 0x400000 : {
		phys = _start;
		*(.text) 
		
	}
	CodeLength = . - phys;
	.data 0x600000 : AT (ADDR (.text) + SIZEOF (.text) )
	{ 
		*(.data)
		*(.rodata*)
	}
	DataLength = SIZEOF (.data);
	.eh_frame : { *(.eh_frame) }
}
