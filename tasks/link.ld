OUTPUT_FORMAT("binary")
ENTRY(_start)
STARTUP(inittask.o)
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
	.bss : 
	{
		*(.bss)
	}
	
	DataLength = SIZEOF (.data) + SIZEOF (.bss);
	.eh_frame : { *(.eh_frame) }
}
