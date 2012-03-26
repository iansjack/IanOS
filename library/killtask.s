	.include "../include/syscalls.inc"

	.global exit

	.text

exit:
	mov $SYS_EXIT, %r9
	syscall
	ret
