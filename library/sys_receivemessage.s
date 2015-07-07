	.include "../include/syscalls.inc"

	.global sys_receivemessage

	.text

sys_receivemessage:
	push %rcx
	push %r9
	push %r11
	mov $RECEIVEMESSAGE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

