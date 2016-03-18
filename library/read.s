	.include "../include/syscalls.inc"
	
	.global read
	.type read, @function

	.text

read:
	push %rcx
	push %r9
	push %r11
	mov $SYS_READ, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

