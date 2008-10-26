	.include "../syscalls.h"

	.global ClearScr

	.text

ClearScr:
	mov $CLEARSCREEN, %r9
	syscall
	ret
