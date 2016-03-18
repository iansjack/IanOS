	.include "../include/syscalls.inc"
	
	.global sys_unlink
	.type sys_unlink,@function

	.text

sys_unlink:
	push %rcx
	push %r9
	push %r11
	mov $SYS_UNLINK, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

