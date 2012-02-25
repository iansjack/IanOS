	.include "../syscalls.inc"

	.global sys_Exit

	.text

sys_Exit:
	mov $SYS_EXIT, %r9
	syscall
	ret
