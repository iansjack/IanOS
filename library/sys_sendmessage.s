	.include "../include/syscalls.inc"

	.global sys_sendmessage

	.text

sys_sendmessage:
	push %rcx
	push %r9
	push %r11
	mov $SENDMESSAGE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

