	.include "../include/syscalls.inc"
	
	.global sys_time

	.text

sys_time:
	push %rcx
	push %r9
	push %r11
	mov $SYS_TIME, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

