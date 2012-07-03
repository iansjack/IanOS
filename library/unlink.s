	.include "../include/syscalls.inc"
	
	.global unlink

	.text

unlink:
	push %rcx
	push %r9
	push %r11
	mov $SYS_UNLINK, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

