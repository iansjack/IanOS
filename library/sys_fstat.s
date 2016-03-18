	.include "../include/syscalls.inc"
	
	.global sys_fstat
	.type sys_fstat, @function

	.text

sys_fstat:
	push %rcx
	push %r9
	push %r11
	mov $SYS_FSTAT, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

