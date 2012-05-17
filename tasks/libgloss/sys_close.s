	.include "../../include/syscalls.inc"
	
	.global sys_close

	.text

sys_close:
	push %rcx
	push %r9
	push %r11
	mov $SYS_CLOSE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

