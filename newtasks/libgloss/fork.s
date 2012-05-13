	.include "../../include/syscalls.inc"
	
	.global fork

	.text

fork:
	mov $SYS_FORK, %r9
	syscall
	ret

