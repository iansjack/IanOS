	.include "../syscalls.inc"

	.global sys_CreateLPTask

	.text

sys_CreateLPTask:
	mov $CREATELPTASK, %r9
	syscall
	ret
