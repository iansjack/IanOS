	.include "../include/syscalls.inc"
	
	.global getcwd

	.text

getcwd:
	push %rcx
	push %r9
	push %r11
	mov $SYS_GETCWD, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

