	.include "../include/syscalls.inc"
	
	.global Alloc_Shared_Page;

	.text

Alloc_Shared_Page:
	push %rcx
	push %r9
	push %r11
	mov $ALLOCSHAREDPAGE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

