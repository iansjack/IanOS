	.include "../syscalls.inc"

	.global sys_ReceiveMessage
	.global sys_SendMessage
	.global sys_SendReceive
	.global sys_WriteDouble
	.global sys_WriteString
	.global sys_AllocMessagePort
	.global sys_GetCurrentConsole
	.global sys_GetCurrentDirectory
	.global sys_SetCurrentDirectory

	.text

sys_ReceiveMessage:
	mov $RECEIVEMESSAGE, %r9
	syscall
	ret

sys_SendMessage:
	mov $SENDMESSAGE, %r9
	syscall
	ret

sys_SendReceive:
	mov $SENDRECEIVE, %r9
	syscall
	ret

sys_WriteDouble:
	push %rbx
	movq %rsi, %rbx
	movb %bl, %bh
	movb %dl, %bl
	movq %rdi, %rdx
	movq $PRINTDOUBLE, %r9
	syscall
	pop %rbx
	ret

sys_WriteString:
	push %rbx
	movq %rsi, %rbx
	movb %bl, %bh
	movb %dl, %bl
	movq %rdi, %rdx
	movq $PRINTSTRING, %r9
	syscall
	pop %rbx
	ret

sys_AllocMessagePort:
	mov $ALLOCMSGPORT, %r9
	syscall
	ret

sys_GetCurrentConsole:
	mov $GETCURRENTCONSOLE, %r9
	syscall
	ret

sys_GetCurrentDirectory:
	mov $GETCURRENTDIR, %r9
	syscall
	ret

sys_SetCurrentDirectory:
	mov $SETCURRENTDIR, %r9
	syscall
	ret
