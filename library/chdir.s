	.include "../include/syscalls.inc"
	
	.global chdir
	.text

chdir:
	push %rcx
	push %r9
	push %r11
	mov $SYS_CHDIR, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

