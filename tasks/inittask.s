#KernelStack  = 0x00000000003FC000
#UserStack    = 0x00000000003FE000

#   .section        .rodata.str1.1,"aMS",@progbits,1
#H:                      .ascii  "IJ64"
#
	.text

	.global _start

_start:		call main
Magic:		.ascii	"IJ64"
CodeLen:	.quad	CodeLength
DataLen:	.quad	DataLength
