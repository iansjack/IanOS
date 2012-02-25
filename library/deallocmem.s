	.include "../include/syscalls.inc"

	.global sys_DeallocMem

	.text

sys_DeallocMem:
	mov $DEALLOCMEM, %r9
	syscall
	ret
