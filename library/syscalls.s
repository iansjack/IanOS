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
