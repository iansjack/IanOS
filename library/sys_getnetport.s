	.include "../include/syscalls.inc"

	.global sys_getnetport
	.text

sys_getnetport:
	push %rcx
	push %r9
	push %r11
	mov $SYS_GETNETPORT, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

