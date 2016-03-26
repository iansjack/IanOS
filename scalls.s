	.include "include/syscalls.inc"

	.global fork
	.global execve
	.global nanosleep
	.global exit
	.global endofsyscalls

	.text
	
# Some system calls
fork:
	mov $SYS_FORK, %r9
	syscall
	ret

execve:
	mov $SYS_EXECVE, %r9
	syscall
	ret

nanosleep:
	mov $SYS_NANOSLEEP, %r9
	syscall
	ret

exit:
	mov $SYS_EXIT, %r9
	syscall
	ret

endofsyscalls:

