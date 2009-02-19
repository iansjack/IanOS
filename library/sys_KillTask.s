	.include "../syscalls.h"

	.global sys_KillTask
	.global sys_Halt

	.text

sys_KillTask:
	mov $KILLTASK, %r9
	syscall
	ret

sys_Halt:
	mov $HALT, %r9
	syscall
	ret

