	.include "../../include/syscalls.inc"
	
	.global read

	.text

read:
	mov $SYS_READ, %r9
	syscall
	ret

