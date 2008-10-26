	section '.text' executable

	public AllocMessagePort
	public SendMessage
	public ReceiveMessage

AllocMessagePort:
	mov  rax, MP.size
	call AllocKMem				  	; allocate memory for message port
	mov  r14, rax				  	; RBX now contains pointer to port
	mov  [MP.waitingProc], 0xFFFFFFFFFFFFFFFF 	; not waiting on a process
	mov  [MP.msgQueue], 0
	ret

;===================================
; Send a message to a message port.
; RAX = message port
; RBX = message
;===================================
SendMessage:
	push r13
	push r14
	mov  r14, rax
	; Allocate some kernel memory to copy the message to ...
	mov  rax, Msg.size
	call AllocKMem
	mov  r13, rax
	; ... and copy the message
	push rcx
	push rsi
	push rdi
	mov  rcx, Msg.size
	mov  rdi, r13
	mov  rsi, rbx
	cld
	rep  movsb
	pop  rdi
	pop  rsi
	pop  rcx
	; Attach the message to the end of the message queue
	cmp  [MP.msgQueue], 0
	jne  .thereIsAMsgQ
	; There is no current message queue, so create one using the message
	mov  [MP.msgQueue], r13
	jmp  .isTaskWaiting
.thereIsAMsgQ:
	push r13
	mov  r13, [MP.msgQueue]
.again: cmp  [Msg.nextMessage], 0
	je   .endOfMessageQ
	mov  r13, [Msg.nextMessage]
	jmp  .again
.endOfMessageQ:
	pop  [Msg.nextMessage]				; This is the address of the message, which we pushed as R13
	; Is a task waiting on this message port?
.isTaskWaiting:
	cmp  [MP.waitingProc], 0xFFFFFFFFFFFFFFFF
	je   .notwaiting
	mov  r15, 0xFFFFFFFFFFFFFFFF
	xchg r15, [MP.waitingProc]			; Retrieve the waiting process and free the port
	mov  [TS.waiting], 0				; Mark the waiting process as free to run
	SWITCH_TASKS_R15
.notwaiting:
	pop r14
	pop r13
	retq

;===========================================================
; Receive a message on message port RAX
; RBX = Buffer to return message to
; If there is no message on the port block and wait for one
; Return message in RAX
;===========================================================
ReceiveMessage:
	push r13
	push r14
	mov  r14, rax
	cmp  [MP.msgQueue], 0	    			; Is there at least one message waiting on the port
	jne  .messageWaiting
	; There's no message waiting so set this task to wait at the port until a message arrives
	mov  r15, [currentTask]
	mov  [MP.waitingProc], r15
	mov  [TS.waiting], 0x80     			; 0x80 is just an arbitrary value at the moment!
	SWITCH_TASKS		    			; This will block until a message arrives at the port
.messageWaiting:
	; Unlink the first message from the waiting queue
	mov  r13, [MP.msgQueue]     			; Get the address of the first message in the waiting queue
	mov  rax, [Msg.nextMessage]
	mov  [MP.msgQueue], rax
	; Copy the waiting message to the buffer perovided by the task
	push rcx
	push rsi
	push rdi
	mov  rcx, Msg.size
	mov  rsi, r13
	mov  rdi, rbx
	cld
	rep  movsb
	pop  rdi
	pop  rsi
	pop  rcx
	pop r14
	pop r13
	retq