	.include "../include/syscalls.inc"
	
	.global getpid

	.text

getpid:
	push %rcx
	push %r9
	push %r11
	mov $SYS_GETPID, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

