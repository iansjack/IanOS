	.include "../syscalls.inc"

	.global sys_Sleep

	.text

sys_Sleep:
	mov $SLEEP, %r9
	syscall
	ret
