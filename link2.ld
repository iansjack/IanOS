OUTPUT_FORMAT("elf64-x86-64")
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
	}
}
