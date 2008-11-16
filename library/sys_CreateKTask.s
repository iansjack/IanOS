	.include "../syscalls.h"

	.global sys_CreateKTask

	.text

sys_CreateKTask:
	mov $CREATEKTASK, %r9
	syscall
	ret
