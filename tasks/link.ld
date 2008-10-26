OUTPUT_FORMAT("binary")
ENTRY(_start)
STARTUP(inittask.o)
SEARCH_DIR(.)
SECTIONS
{
	.text 0x300000 : {
		phys = _start;
		*(.text) 
		
	}
	CodeLength = . - phys;
	.data 0x310000 : AT (ADDR (.text) + SIZEOF (.text) )
	{ 
		*(.data)
		*(.rodata*)
	}
	DataLength = SIZEOF (.data);
	.eh_frame : { *(.eh_frame) }
}
