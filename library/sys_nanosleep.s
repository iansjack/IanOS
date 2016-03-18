	.include "../include/syscalls.inc"
	
	.global sys_nanosleep
	.type sys_nanosleep,@function

	.text

sys_nanosleep:
	push %rcx
	push %r9
	push %r11
	mov $SYS_NANOSLEEP, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

