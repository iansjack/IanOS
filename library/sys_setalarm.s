	.include "../include/syscalls.inc"
	
	.global sys_alarm
	.type sys_alarm,@function

	.text

sys_alarm:
	push %rcx
	push %r9
	push %r11
	mov $SYS_ALARM, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

