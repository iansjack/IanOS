	.include "../../include/syscalls.inc"
	
	.global sys_fstat

	.text

sys_fstat:
	mov $SYS_FSTAT, %r9
	syscall
	ret

