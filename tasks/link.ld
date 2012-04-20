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
	.rodata :
	{
		*(.rodata)
	}
	CodeLength = . - phys;
	.data 0x600000 : AT (ADDR (.text) + SIZEOF (.text) )
	{ 
		*(.data)
		*(.bssdata)
	}
	
	DataLength = SIZEOF (.data) + SIZEOF (.bss);
}
