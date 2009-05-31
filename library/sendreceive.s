	.include "../syscalls.inc"

	.global SendReceive

	.text

SendReceive:
	mov $SENDRECEIVE, %r9
	syscall
	ret
