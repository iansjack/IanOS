	.include "../../include/syscalls.inc"
	
	.global execve

	.text

execve:
	mov $SYS_EXECVE, %r9
	syscall
	ret

