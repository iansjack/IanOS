	.include "../syscalls.inc"

	.global sys_AllocMem
	.global sys_AllocSharedMem
	.global sys_CreateKTask
	.global sys_CreateLPTask
	.global sys_GetCommandLine
	.global sys_CreateTask
	.global sys_DeallocMem
	.global sys_KillTask
	.global sys_ReceiveMessage
	.global sys_SendMessage
	.global sys_SendReceive
	.global sys_Sleep
	.global sys_GetTicks
	.global sys_WriteDouble
	.global sys_WriteString
	.global sys_AllocMessagePort
	.global sys_GetCurrentConsole
	.global sys_GetCurrentDirectory
	.global sys_SetCurrentDirectory

	.text

sys_AllocMem:
	mov $ALLOCMEM, %r9
	syscall
	ret

sys_AllocSharedMem:
	mov $ALLOCSHAREDMEM, %r9
	syscall
	ret

sys_CreateKTask:
	mov $CREATEKTASK, %r9
	syscall
	ret

sys_CreateLPTask:
	mov $CREATELPTASK, %r9
	syscall
	ret

sys_GetCommandLine:
	mov $GETCOMMANDLINE, %r9
	syscall
	ret

sys_CreateTask:
	mov $CREATETASK, %r9
	mov %rcx, %r14
	syscall
	ret

sys_DeallocMem:
	mov $DEALLOCMEM, %r9
	syscall
	ret

sys_KillTask:
	mov $KILLTASK, %r9
	syscall
	ret

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

sys_Sleep:
	mov $SLEEP, %r9
	syscall
	ret

sys_GetTicks:
	mov $GETTICKS, %r9
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
