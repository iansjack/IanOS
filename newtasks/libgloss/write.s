	.include "../../include/syscalls.inc"
	
	.global write

	.text

write:
	push %rcx
	push %r9
	push %r11
	mov $SYS_WRITE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

