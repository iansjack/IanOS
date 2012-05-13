	.include "../../include/syscalls.inc"
	
	.global chdir
	.text

chdir:
	mov $SYS_CHDIR, %r9
	syscall
	ret

