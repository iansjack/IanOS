OUTPUT_FORMAT("binary")
ENTRY(_start)
SECTIONS
{
	. = 0x160000;
	.text : { *(.text)  }
	. = 0x1A8000;
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
