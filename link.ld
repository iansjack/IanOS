OUTPUT_FORMAT("binary")
ENTRY(_start)
SECTIONS
{
	. = 0x1000;
	.text : { *(.text)  }
	. = 0x10000;
	.data : 
	{ 
	*(.data) 
	*(.rodata)
	*(.bssdata)
	}
}
