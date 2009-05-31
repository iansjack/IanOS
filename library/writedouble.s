	.include "../syscalls.inc"

	.global WriteDouble

	.text

WriteDouble:
	push %rbx
	movq %rsi, %rbx
	movb %bl, %bh
	movb %dl, %bl
	movq %rdi, %rdx
	movq $PRINTDOUBLE, %r9
	syscall
	pop %rbx
	ret
