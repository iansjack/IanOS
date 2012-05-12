	.include "../../include/syscalls.inc"
	
	.global close

	.text

close:
	mov $SYS_CLOSE, %r9
	syscall
	ret

