	.include "../syscalls.inc"

	.global sys_AllocMem
	.global sys_AllocSharedMem

	.text

sys_AllocMem:
	mov $ALLOCMEM, %r9
	syscall
	ret

sys_AllocSharedMem:
	mov $ALLOCSHAREDMEM, %r9
	syscall
	ret
