	.include "../syscalls.h"

	.global Send

	.text

Send:
	mov $SENDMESSAGE, %r9
	syscall
	ret
