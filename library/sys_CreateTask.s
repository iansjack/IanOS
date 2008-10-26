	.include "../syscalls.h"

	.global sys_CreateTask

	.text

sys_CreateTask:
	mov $CREATETASK, %r9
	syscall
	ret
