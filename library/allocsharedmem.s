	.include "../include/syscalls.inc"

	.global sys_AllocSharedMem

	.text

sys_AllocSharedMem:
	mov $ALLOCSHAREDMEM, %r9
	syscall
	ret
