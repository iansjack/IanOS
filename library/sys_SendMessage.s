	.include "../syscalls.inc"

	.global sys_SendMessage

	.text

sys_SendMessage:
	mov $SENDMESSAGE, %r9
	syscall
	ret
