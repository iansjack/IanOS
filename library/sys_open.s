	.include "../include/syscalls.inc"
	
	.global sys_open

	.text

sys_open:
	push %rcx
	push %r9
	push %r11
	mov $SYS_OPEN, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

