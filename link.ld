OUTPUT_FORMAT("binary")
ENTRY(_start)
SECTIONS
{
	. = 0x10000;
	.text : { *(.text)  }
	. = 0x20000;
	.data : 
	{ 
		*(.data) 
		*(.rodata)
		*(.bssdata)
	}
	/DISCARD/ :
	{
		*(.comment)
		*(.eh_frame)
	}
}
