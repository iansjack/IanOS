	.include "../../include/syscalls.inc"
	.global _exit;

	.text

_exit:
	push %rcx
	push %r9
	push %r11
	mov $SYS_EXIT, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

