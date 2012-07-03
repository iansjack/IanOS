	.include "../include/syscalls.inc"
	
	.global sys_execve

	.text

sys_execve:
	push %rcx
	push %r9
	push %r11
	mov $SYS_EXECVE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

