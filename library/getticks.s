	.include "../syscalls.inc"

	.global GetTicks

	.text

GetTicks:
	mov $GETTICKS, %r9
	syscall
	ret
