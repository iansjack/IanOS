;include 'memory.h'
;include 'syscalls.h'
;include 'macros.asm'

;	format ELF64

	section '.text' executable

tas1:
	org UserCode

;=================
; Code for Task 1
;=================
	mov qword [UserData], 0
	mov qword [UserData + 8], 0xFFD
	SYS CLEARSCREEN
	WRITE_STRING message, 10, 0
	mov rsi, T2Name
	SYS CREATETASK
.again: mov rax, KbdPort
	mov [Msg1.byte], 1		; Command to read a character from the keyboard
	mov rbx, Msg1
	SYS SENDRECEIVE
	mov al, [Msg1.byte]
	cmp al, 0
	je .nowrite
	WRITE_CHAR al, 11, 0
	cmp al, 't'
	jne .nott
	mov rsi, T3Name
	SYS CREATETASK
.nott:	cmp al, 's'
	jne .nots
	mov rax, 200
	SYS SLEEP
.nots:	cmp al, 'm'
	jne .notm
	mov rax, 123
	SYS ALLOCMEM
.notm:	cmp al, 'p'
	jne .nowrite
	mov rax, Msg.size
	SYS ALLOCMEM
	mov r13, rax
	mov [Msg.byte], 0x12
	mov rax, 0x98765432FFFFFFFF
	mov [Msg.quad], rax
	mov rbx, r13
	mov rax, StaticPort
	SYS SENDMESSAGE
	mov rax, r13
	SYS DEALLOCMEM
.nowrite:
	SYS GETTICKS
	WRITE_DOUBLE eax, 20, 60
	jmp .again

T2Name	db 'TASK2   BIN'
T3Name	db 'TASK3   BIN'
Port1	dq 0
Msg1 Message

	org $ - UserCode + tas1

taskend: