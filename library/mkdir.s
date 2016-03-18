	.include "../include/syscalls.inc"
	
	.global mkdir
	.type mkdir, @function

	.text

mkdir:
	push %rcx
	push %r9
	push %r11
	mov $SYS_MKDIR, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

