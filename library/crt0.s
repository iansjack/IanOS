	.text

	.global _start
	.global DataLen
	.extern main

_start:		call main
			call _exit

Magic:		.ascii	"IJ64"
#CodeLen:	.quad	CodeLength
#DataLen:	.quad	DataLength
