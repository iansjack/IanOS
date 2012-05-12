	.include "../../include/syscalls.inc"
	
	.global unlink

	.text

unlink:
	mov $SYS_UNLINK, %r9
	syscall
	ret

