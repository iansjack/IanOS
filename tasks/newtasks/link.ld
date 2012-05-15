OUTPUT_FORMAT("binary")
ENTRY(_start)
SEARCH_DIR(.)
SECTIONS
{
	.text 0x400000 : 
	{
		phys = _start;
		*(.text)
	}
	.rodata :
	{
		*(.rodata*)
	}
	CodeLength = . - phys;
	.data 0x600000 : AT (ADDR (.text) + CodeLength)
	{ 
		*(.data)
		*(.bssdata)
	}
	/DISCARD/ :
	{
		*(.comment)
		*(.eh_frame)
		*(.fini)
		*(.init)
		*(.debug*)
	}
	
	DataLength = . - 0x600000;
	CodeEnd = .;
}
