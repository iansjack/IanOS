	.include "../../include/syscalls.inc"
	
	.global creat

	.text

creat:
	mov $SYS_CREAT, %r9
	syscall
	ret

