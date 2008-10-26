	.text

InBuffEmpty:
	push %eax
	push %ecx
	mov $0x2FFFF, %ecx
ibe:	in $0x64, %al
	test $2, %al
	loopnz ibe
	pop %ecx
	pop %eax
	ret

OutBuffFull:
	push %eax
	push %ecx
	mov $0x2FFFF, %ecx
obf:	in $0x64, %al
	test $1, %al
	loopz	obf
	pop %ecx
	pop %eax
	ret
