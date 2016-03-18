	.include "../include/syscalls.inc"

	.global sys_receivepacket
	.type sys_receivepacket, @function

	.text

sys_receivepacket:
	push %rcx
	push %r9
	push %r11
	mov $SYS_RECEIVEPACKET, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

