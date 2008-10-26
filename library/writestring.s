	.include "../syscalls.h"

	.global WriteString

	.text

WriteString:
	push %rbx
	movq %rsi, %rbx
	movb %bl, %bh
	movb %dl, %bl
	movq %rdi, %rdx
	movq $PRINTSTRING, %r9
	syscall
	pop %rbx
	ret
