	.include "../include/syscalls.inc"
	
	.global lseek
	.type lseek, @function

	.text

lseek:
	push %rcx
	push %r9
	push %r11
	mov $SYS_LSEEK, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

