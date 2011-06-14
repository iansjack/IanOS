	.include "../syscalls.inc"

	.global sys_GetCommandLine

	.text

sys_GetCommandLine:
	mov $GETCOMMANDLINE, %r9
	syscall
	ret
