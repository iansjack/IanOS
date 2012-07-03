	.include "../include/syscalls.inc"
	
	.global Alloc_Page;

	.text

Alloc_Page:
	push %rcx
	push %r9
	push %r11
	mov $ALLOCPAGE, %r9
	syscall
	pop %r11
	pop %r9
	pop %rcx
	ret

