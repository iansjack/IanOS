	.include "../../include/syscalls.inc"
	
	.global write

	.text

write:
	mov $SYS_WRITE, %r9
	syscall
	ret

