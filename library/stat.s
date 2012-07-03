	.include "../include/syscalls.inc"
	
	.global stat

	.text
	
stat:
	push %rcx
	push %r9
	push %r11
	mov $SYS_STAT, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret
		

