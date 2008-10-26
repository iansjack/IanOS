include 'syscalls.h'
include 'memory.h'
include 'macros.asm'
;include 'kstructs.asm'

	use64
Magic	db	'IJ64'
CodeLen dq	CodeLength
DataLen dq	DataLength

;=================
; Code for Task 3
;=================
task3code:
	org UserCode

	SETUP_STACKS 3
	WRITE_STRING t3message, 13, 0
	mov rdi, Msg.size
	SYS ALLOCMEM				    ; Allocate memory for the message
	;mov rbx, rax
	;mov rax, StaticPort
	mov  rsi, rax
	mov  rdi, StaticPort
	SYS RECEIVEMESSAGE			    ; Receive a message into [RBX] from StaticPort
	WRITE_STRING messagereceived, 14, 0
	mov r13, rsi
	WRITE_DOUBLE dword [Msg.quad + 4], 14, 60
	WRITE_DOUBLE dword [Msg.quad], 14, 68
.again: jmp .again

CodeLength = $ - UserCode

	org	UserData

t3message db 'Task 3 is now running', 0
messagereceived db 'Got a message from Task 1', 0

DataLength = $ - UserData