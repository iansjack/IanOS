	section '.text'

;===================
; Keyboard interrupt
;===================
KbInt:	push rax
	push rbx
	in   al, 0x60		 ; MUST read byte from keyboard - else no more ints
	test al, 0x80		 ; only interested in key make events
	jnz  .done
	mov  ebx,KbdTable
	xlat [ebx]
	mov  ebx, kbBuffer
	add  ebx, [kbBufCurrent]
	mov  [ebx], al
	inc  [kbBufCurrent]
	inc  [kbBufCount]
	cmp  [kbBufCurrent], 128
	jne  .istaskwaiting
	mov  [kbBufCurrent], 0
	; is any task waiting for keyboard input? If so re-enable it
.istaskwaiting:
	mov  r15, [currentTask]
.again: cmp  [TS.waiting], KBDINT
	jne  .goon
	mov  [TS.waiting], 0
	SWITCH_TASKS_R15
	jmp  .done
.goon:	mov  r15, [TS.nexttask]
	cmp  r15, [currentTask]
	jne  .again
.done:	pop  rbx
	mov  al, 0x20		  ; clear int
	out  0x20, al
	pop  rax
	iretq

;==========================================
; Code for Task 4 (Keyboard Device Driver)
;==========================================
kbTaskCode:
	KSETUP_STACKS
	KWRITE_STRING t4message, 15, 0
.again: mov  rax, KbdPort
	mov  rbx, KbdMsg
	call ReceiveMessage
	cmp [KbdMsg.byte], 1
	jne .unknownCommand
	cmp [kbBufCount], 0
	jne .keyavail
	WAIT_FOR_INT KBDINT
.keyavail:
	mov ebx, kbBuffer
	add ebx, [kbBufStart]
	mov bl, [ebx]
	inc [kbBufStart]
	dec [kbBufCount]
	cmp [kbBufStart], 128
	jne .done
	mov [kbBufStart], 0
.done:	mov rax, [KbdMsg.quad]		   ; Port to reply on
	mov [KbdMsg.quad], 0
	mov [KbdMsg.byte], bl
	mov rbx, KbdMsg
	call SendMessage
	jmp .again
.unknownCommand:
	mov rax, [KbdMsg.quad]
	mov [KbdMsg.quad], 1
	mov [KbdMsg.byte], 0

	section '.data'
t4message db 'Keyboard task is now running', 0
KbdMsg Message
kbBufStart   dd 0
kbBufCurrent dd 0
kbBufCount   db 0
kbBuffer:
	db 128 dup (?),0

include 'keytabs.asm'
