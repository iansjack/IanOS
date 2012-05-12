	.include "../../include/syscalls.inc"
	
	.global open

	.text

open:
	mov $SYS_OPEN, %r9
	syscall
	ret

