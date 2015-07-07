	.include "../include/syscalls.inc"

	.global sys_queuepacket

	.text

sys_queuepacket:
	push %rcx
	push %r9
	push %r11
	mov $SYS_QUEUEPACKET, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

