	.include "../syscalls.h"

	.global WriteChar

	.text

WriteChar:
	push %rbx
	movq %rsi, %rbx
	movb %bl, %bh
	movb %dl, %bl
	movq %rdi, %rax
	movb %al, %ah
	movq $PRINTCHAR, %r9
	syscall
	pop %rbx
	ret
