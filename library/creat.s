	.include "../include/syscalls.inc"
	
	.global creat
	.type creat, @function

	.text

creat:
	push %rcx
	push %r9
	push %r11
	mov $SYS_CREAT, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

