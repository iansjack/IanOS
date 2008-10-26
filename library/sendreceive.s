	.include "../syscalls.h"

	.global SendReceive

	.text

SendReceive:
	mov $SENDRECEIVE, %r9
	syscall
	ret
