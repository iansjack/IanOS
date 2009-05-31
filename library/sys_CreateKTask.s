	.include "../syscalls.inc"

	.global sys_CreateKTask
	.global sys_CreateLPTask
	.global sys_GetCommandLine

	.text

sys_CreateKTask:
	mov $CREATEKTASK, %r9
	syscall
	ret

sys_CreateLPTask:
	mov $CREATELPTASK, %r9
	syscall
	ret

sys_GetCommandLine:
	mov $GETCOMMANDLINE, %r9
	syscall
	ret
