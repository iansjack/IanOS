	.include "../../include/syscalls.inc"
	
	.global Alloc_Page;

	.text

Alloc_Page:
	mov $ALLOCPAGE, %r9
	syscall
	ret

