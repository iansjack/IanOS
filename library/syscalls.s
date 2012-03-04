	.include "../include/syscalls.inc"
	
	.global sys_ReceiveMessage
	.global sys_SendMessage
	.global sys_SendReceive
	.global sys_GetCurrentConsole
	.global sys_GetCurrentDirectory
	.global sys_SetCurrentDirectory
	.global Sys_Open
	.global Sys_Close
	.global Sys_Fork
	.global Sys_Execve
	.global Sys_Wait
	.global Sys_Stat
	.global Sys_Read
	.global Sys_Write
	.global Sys_Creat

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
	
Sys_Open:
	mov $SYS_OPEN, %r9
	syscall
	ret

#SYS Sys_Open, SYS_OPEN

Sys_Close:
	mov $SYS_CLOSE, %r9
	syscall
	ret
	
Sys_Fork:
	mov $SYS_FORK, %r9
	syscall
	ret
	
Sys_Execve:
	mov $SYS_EXECVE, %r9
	syscall
	ret

Sys_Wait:
	mov $SYS_WAITPID, %r9
	syscall
	ret
	
Sys_Stat:
	mov $SYS_STAT, %r9
	syscall
	ret
		
Sys_Read:
	mov $SYS_READ, %r9
	syscall
	ret

Sys_Write:
	mov $SYS_WRITE, %r9
	syscall
	ret

Sys_Creat:
	mov $SYS_CREAT, %r9
	syscall
	ret
	