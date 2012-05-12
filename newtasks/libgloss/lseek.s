	.include "../../include/syscalls.inc"
	
	.global lseek

	.text

lseek:
	mov $SYS_LSEEK, %r9
	syscall
	ret

