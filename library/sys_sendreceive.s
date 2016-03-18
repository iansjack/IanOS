	.include "../include/syscalls.inc"

	.global sys_sendreceive
	.type sys_sendreceive, @function

	.text

sys_sendreceive:
	push %rcx
	push %r9
	push %r11
	mov $SENDRECEIVE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

