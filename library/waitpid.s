	.include "../include/syscalls.inc"
	
	.global waitpid
	.type waitpid,@function

	.text

waitpid:
	push %rcx
	push %r9
	push %r11
	mov $SYS_WAITPID, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

