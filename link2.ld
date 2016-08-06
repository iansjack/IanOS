OUTPUT_FORMAT("elf64-x86-64")
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
	}
}
