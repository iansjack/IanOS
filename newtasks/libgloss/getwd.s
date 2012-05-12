	.include "../../include/syscalls.inc"
	
	.global getwd

	.text

getwd:
	mov $SYS_GETCWD, %r9
	syscall
	ret

