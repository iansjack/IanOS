	.include "../include/syscalls.inc"

	.global sys_CreateTask

	.text

sys_CreateTask:
	mov $CREATETASK, %r9
	mov %rcx, %r14
	syscall
	ret
