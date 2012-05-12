	.include "../../include/syscalls.inc"
	
	.global mkdir

	.text

mkdir:
	mov $SYS_MKDIR, %r9
	syscall
	ret

