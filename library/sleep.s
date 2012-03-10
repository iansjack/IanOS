	.include "../include/syscalls.inc"

	.global sys_Sleep

	.text

sys_Sleep:
	mov $SYS_NANOSLEEP, %r9
	syscall
	ret
