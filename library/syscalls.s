	.include "../include/syscalls.inc"
	
	.global sys_ReceiveMessage
	.global sys_SendMessage
	.global sys_SendReceive
	.global sys_GetCurrentConsole
	.global getcwd
	.global chdir
	.global open
	.global close
	.global fork
	.global execve
	.global wait
	.global stat
	.global read
	.global write
	.global creat
	.global unlink
	.global nanosleep
	.global mkdir
	.global lseek

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
	mov $SYS_GETCWD, %r9
	syscall
	ret

getcwd:
	mov $SYS_GETCWD, %r9
	syscall
	ret

chdir:
	mov $SYS_CHDIR, %r9
	syscall
	ret
	
open:
	mov $SYS_OPEN, %r9
	syscall
	ret

close:
	mov $SYS_CLOSE, %r9
	syscall
	ret
	
fork:
	mov $SYS_FORK, %r9
	syscall
	ret
	
execve:
	mov $SYS_EXECVE, %r9
	syscall
	ret

wait:
	mov $SYS_WAITPID, %r9
	syscall
	ret
	
stat:
	mov $SYS_STAT, %r9
	syscall
	ret
		
read:
	mov $SYS_READ, %r9
	syscall
	ret

write:
	mov $SYS_WRITE, %r9
	syscall
	ret

creat:
	mov $SYS_CREAT, %r9
	syscall
	ret
	
unlink:
	mov $SYS_UNLINK, %r9
	syscall 
	ret

nanosleep:
	mov $SYS_NANOSLEEP, %r9
	syscall
	ret
	
mkDir:
	mov $SYS_MKDIR, %r9
	syscall
	ret
	
lseek:
	mov $SYS_LSEEK, %r9
	syscall
	ret

