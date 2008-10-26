	section '.text' executable

CallNo	dq	AllocatePage64		; ALLOCPAGE
	dq	PrintString		; PRINTSTRING
	dq	PrintDouble		; PRINTDOUBLE
	dq	PrintChar		; PRINTCHAR
	dq	GetKey			; GETKEY
	dq	NewTask 		; NEWTASK
	dq	ClearScreen		; CLEARSCREEN
	dq	GetTicks		; GETTICKS
	dq	Sleep			; SLEEP
	dq	LoadAFile		; LOADFILE
	dq	Alloc_Mem		; ALLOCMEM
	dq	AllocKernelMemory	; ALLOCKMEM             Do we need this one?
	dq	Alloc_Message_Port	; ALLOCMSGPORT
	dq	CreateMessage		; CREATEMESSAGE
	dq	Send_Message		; SENDMESSAGE
	dq	Receive_Message 	; RECEIVEMESSAGE
	dq	Dealloc_Mem		; DEALLOCMEM
	dq	Send_Receive		; SENDRECEIVE

SysCalls:

	jmp [CallNo + 8 * r9 - 8]

;===========================================
; Allocate a page of memory (64-bit version)
; Create a PTE for it pointing to RBX
; return in RAX the Physical Memory address
;===========================================
AllocatePage64:
	call AllocPage64
	call CreatePTE			  ; map this page to [RBX]
	sysret

;========================================================
; Print [EDX] as string at position row BH col BL
; Affects RAX, RBX, RDX
;========================================================
PrintString:
	mov ax, 160
	mul bh
	mov bh, 0
	shl bx, 1
	add bx, ax
.isItStringEnd:
	mov ah, [edx]
	cmp ah, 0
	je .done
	mov [ebx + 0xB8000], ah
	add bx,2
	inc edx
	jmp .isItStringEnd
.done:	sysret

;========================================================
; Print EDX as hex at position row BH col BL
; Affects RAX, RBX, RDX
;========================================================
PrintDouble:
	push rcx
	mov ax, 160
	mul bh
	mov bh, 0
	shl bx, 1
	add bx, ax
	mov rcx, 8
.stillCounting:
	shld eax, edx, 4
	shl edx, 4
	and eax, 0xF
	add al, '0'
	cmp al, '9'
	jle .under10
	add al, 7
.under10:
	mov  [ebx + 0xB8000], al
	add  bx,2
	loop .stillCounting
	pop  rcx
	sysret

;========================================================
; Print character in AH at position row BH col BL
; Affects RAX, RBX
;========================================================
PrintChar:
	push ax
	mov  ax, 160
	mul  bh
	mov  bh, 0
	shl  bx, 1
	add  bx, ax
	pop  ax
	mov  [ebx + 0xB8000],ah
	sysret

;============================================================================
; GetKey. if AL <> 0, and no key is available, then block, otherwise return 0
; Return key (or 0) in AL
; Affects RBX
;============================================================================
GetKey:
	;mov ah, al
	;mov al, 0
	;cmp [kbBufCount], 0
	;jne .keyavail
	;cmp ah, 0
	;je  .done
	;WAIT_FOR_INT KBDINT
;.keyavail:
	;mov ebx, keybBuffer
	;add ebx, [kbBufStart]
	;mov al, [ebx]
	;inc [kbBufStart]
	;dec [kbBufCount]
	;cmp [kbBufStart], 128
	;jne .done
	;mov [kbBufStart], 0
.done:	sysret

;=================================================================
; Create a new task from the file whose name is pointed to by RSI
; Affects RAX, RDI
;=================================================================
NewTask:
	call LoadFile
	push rcx
	cli
	mov  rax, tempstack - 5*8
	mov  r15, [nextfreetss]
	mov  [TS.rsp], rax
	mov  qword [rax], UserCode
	mov  qword [rax + 8], user64 + 3
	mov  qword [rax + 0x10], 0x2202
	mov  qword [rax + 0x18], UserData + 0x1000
	mov  qword [rax + 0x20], udata64 + 3
	call VCreatePageDir
	mov  [TS.waiting], 0		; Make task runnable
	mov  [TS.cr3], rax		; Fill in a few fields in the tss
	mov  [TS.ds], udata64+3
	; Link the new task into the list of tasks
	push r15
	mov  r15, [currentTask]
	mov  rax, [TS.nexttask]
	mov  rbx, [nextfreetss]
	mov  [TS.nexttask], rbx
	pop  r15
	mov  [TS.nexttask], rax
	mov  [TS.firstfreemem], UserData
	add  [nextfreetss], TS.size
	; Now move the code an data to the correct place
	mov  rax, [qword DiskBuffer + 0x4]	; Size of code area
	mov  rcx, rax
	mov  rsi, DiskBuffer + 0x14	; Start of code area
	mov  rdi, TempUserCode
	cld
	rep  movsb			; Move the code to its final location
	mov  rax, [qword DiskBuffer + 0xC]	; Size of data area
	mov  rcx, rax
	add  [TS.firstfreemem], rcx
	mov  rsi, DiskBuffer + 0x14	; Start of code area
	;add  rsi, [qword DiskBuffer + 0x4]	; Add the size of the code area to get to the data area
	mov  rax, [qword DiskBuffer + 0x4]
	add  rsi, rax
	mov  rdi, TempUserData
	cld
	rep  movsb			; Move the data to its final location
	mov  rbx, 0x1000
	mov  rax, [qword DiskBuffer + 0xC]
	sub  rbx, rax
	;sub  rbx, [qword DiskBuffer + 0xC]
	sub  rbx, 0x10
	mov  qword [rdi], 0
	mov  qword [rdi + 0x8], rbx
	pop  rcx
	SWITCH_TASKS
	sysret

;======================
;Clear the console
;======================
ClearScreen:
	push rbx
	push rcx
	mov rbx, 0xB8000
	mov rcx, 0xFA0
.again: mov word [rbx + rcx - 2], 0x0720
	dec rcx
	loop .again
	pop rcx
	pop rbx
	sysret

;=============================================================================
; Return in RAX the number of (10ms) clock ticks since the system was started
;;=============================================================================
GetTicks:
	mov rax, [Ticks]
	sysret

;=====================================================
; Suspend the current task for for RAX 10ms intervals
;=====================================================
Sleep:
	mov [Timer.interval], rax
	mov rax, [currentTask]
	mov [Timer.task], rax
	mov [Timer.active], 1
	WAIT_FOR_INT SLEEPINT
	sysret

;====================================================
; Load a file from the hard disk into the DiskBuffer
; SDI = filename in directory format.
; Currently this assumes that the file is 1 sector
;====================================================
LoadAFile:
	call LoadFile
	sysret

;=====================================================================
; Allocate some memory from the heap. RAX = amount to allocate
; Returns in RAX address of allocated memory.
; Uses a mem structure
; nextmem dq ?  pointer to next mem structure
; size    dq ?  size of this mem structure (including these two quads)
; This simple version just allocates memory from the end of the free
; space list.
;======================================================================
Alloc_Mem:
	push rbx
	mov  r15, [currentTask]
	mov  rbx, [TS.firstfreemem]
	call AllocMem
	pop  rbx
	sysret

;=====================================================================
; Allocate some memory from the Kernel heap. RAX = amount to allocate
;=====================================================================
AllocKernelMemory:
	call AllocKMem
	sysret

;=================================================
; Allocate a message port. Return the port in RAX
;=================================================
Alloc_Message_Port:
	call AllocMessagePort
	sysret

;===============================
; Create a message
; AL = Message.byte
; RBX = Message quad
; Return message address in RAX
;===============================
CreateMessage:
	push rdx
	push rax
	push rbx
	mov  rax, Msg.size
	call AllocKMem
	mov  r13, rax
	mov  [Msg.nextMessage], 0
	pop  [Msg.quad]
	pop  rdx
	mov  [Msg.byte], dl
	pop  rdx
	sysret

;===================================
; Send a message to a message port.
; RAX = message port
; RBX = message
;===================================
Send_Message:
	call SendMessage
	sysret

;===========================================================
; Receive a message on message port RAX
; RBX = Buffer to return message to
; If there is no message on the port block and wait for one
; Return message in RAX
;===========================================================
Receive_Message:
	call ReceiveMessage
	sysret

;==================================================
; Deallocate the memory at location RAX.
; This will deallocate both user and kernel memory
;==================================================
Dealloc_Mem:
	call DeallocMem
	sysret

;======================================================
; Send a message to a message port and receive a reply
; RAX = message port
; RBX = message
;======================================================
Send_Receive:
	push r13
	push r14
	push rax
	mov r13, rbx
	call AllocMessagePort
	mov r14, rax
	mov [Msg.quad], r14		; Port to receive reply on
	pop rax
	mov rbx, r13
	call SendMessage
	mov rax, r14
	mov rbx, r13
	call ReceiveMessage
	pop r14
	pop r13
	sysret
