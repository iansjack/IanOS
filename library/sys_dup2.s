	.include "../include/syscalls.inc"

	.global sys_dup2
	.type sys_dup2, @function

	.text

sys_dup2:
	push %rcx
	push %r9
	push %r11
	mov SYS_DUP2, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

