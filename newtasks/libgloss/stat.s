	.include "../../include/syscalls.inc"
	
	.global stat

	.text
	
stat:
	mov $SYS_STAT, %r9
	syscall
	ret
		

