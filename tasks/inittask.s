KernelStack  = 0x00000000003FC000
UserStack    = 0x00000000003FE000

	.include "../syscalls.h"

	.text

	.global _start

Magic:		.ascii	"IJ64"
CodeLen:	.quad	CodeLength
DataLen:	.quad	DataLength

_start:		call main
