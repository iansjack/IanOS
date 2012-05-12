	.include "../../include/syscalls.inc"
	
	.global getpid

	.text

getpid:
	mov $SYS_GETPID, %r9
	syscall
	ret

