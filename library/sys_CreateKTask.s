	.include "../syscalls.h"

	.global sys_CreateKTask
	.global sys_CreateLPTask

	.text

sys_CreateKTask:
	mov $CREATEKTASK, %r9
	syscall
	ret

sys_CreateLPTask:
	mov $CREATELPTASK, %r9
	syscall
	ret
