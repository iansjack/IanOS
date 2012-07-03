	.include "../include/syscalls.inc"
	
	.global fork

	.text

fork:
	push %rcx
	push %r9
	push %r11
	mov $SYS_FORK, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

