	.include "../../include/syscalls.inc"
	
	.global getcwd

	.text

getcwd:
	mov $SYS_GETCWD, %r9
	syscall
	ret

