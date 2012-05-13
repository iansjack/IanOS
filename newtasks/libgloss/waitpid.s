	.include "../../include/syscalls.inc"
	
	.global waitpid

	.text

waitpid:
	mov $SYS_WAITPID, %r9
	syscall
	ret

