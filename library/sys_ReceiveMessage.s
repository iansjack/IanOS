	.include "../syscalls.inc"

	.global sys_ReceiveMessage

	.text

sys_ReceiveMessage:
	mov $RECEIVEMESSAGE, %r9
	syscall
	ret
