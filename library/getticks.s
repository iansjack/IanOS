	.include "../syscalls.inc"

	.global sys_GetTicks

	.text

sys_GetTicks:
	mov $GETTICKS, %r9
	syscall
	ret
