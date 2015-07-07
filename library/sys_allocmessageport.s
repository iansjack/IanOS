	.include "../include/syscalls.inc"

	.global sys_allocmessageport

	.text

sys_allocmessageport:
	push %rcx
	push %r9
	push %r11
	mov $SYS_ALLOCMESSAGEPORT, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

