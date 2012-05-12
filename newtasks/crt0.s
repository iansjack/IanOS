	.text

	.global _start
	.extern main

_start:		call main
			call _exit

Magic:		.ascii	"IJ64"
CodeLen:	.quad	CodeLength
DataLen:	.quad	DataLength
