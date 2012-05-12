	.include "../../include/syscalls.inc"
	.global _exit;

	.text

_exit:
	mov $SYS_EXIT, %r9
	syscall
	ret

