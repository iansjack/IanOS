	.include "../include/syscalls.inc"
	
	.global sys_truncate

	.text

sys_truncate:
	push %rcx
	push %r9
	push %r11
	mov $SYS_FTRUNCATE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret
