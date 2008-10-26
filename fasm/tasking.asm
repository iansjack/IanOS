	section '.text' executable
;===================================================================================================
; Save the current task, restore the next ready one (if there is one!), and initiate a task switch.
; If there isn't another runnable task, and the current task is still runnable, just continue.
; If there's no runnable task at all then halt the processor until the next interrupt.
;===================================================================================================
TaskSwitch:
;saveTaskState:
	push r15
	mov  r15, [currentTask]
; Is there another task ready to run?
.nexttask:
	mov  r15, [TS.nexttask]
	cmp  r15, [currentTask]
	je   .nonefound
	cmp  [TS.waiting], 0
	jne  .nexttask
	jmp  TS1
.nonefound:
; No other task is runnable. Is the current task runnable? If so just keep running it
	cmp  [TS.waiting], 0
	jne  .notaskrunnable
	pop  r15
	ret
; There's no runnable task, so just idle until the next interrupt
.notaskrunnable:
	sti
	hlt
	cli
	jmp  .nexttask

;==============================================================
; Save the current task and switch to the one specified in R15
;==============================================================
SpecificTaskSwitch:
; Save task state
	push r15
TS1:	xchg r15, [currentTask]
	mov  [TS.rax], rax
	mov  [TS.rbx], rbx
	mov  [TS.rcx], rcx
	mov  [TS.rdx], rdx
	mov  [TS.rbp], rbp
	mov  [TS.rsi], rsi
	mov  [TS.rdi], rdi
	mov  [TS.r8], r8
	mov  [TS.r9], r9
	mov  [TS.r10], r10
	mov  [TS.r11], r11
	mov  [TS.r12], r12
	mov  [TS.r13], r13
	mov  [TS.r14], r14
	pop  rax
	mov  [TS.r15], rax
	mov  [TS.ds], ds
	mov  rax, cr3
	mov  [TS.cr3], rax
	pop  rcx
	mov  [TS.rsp], rsp
; Restore task state
	mov  r15, [currentTask] 	; Actually the task to switch to (which we stored in currentTask)
	mov  rax, [TS.cr3]
	mov  cr3, rax
	mov  rsp, [TS.rsp]
	push rcx
	mov  rcx, [TS.rcx]
	mov  rdx, [TS.rdx]
	mov  rbp, [TS.rbp]
	mov  rsi, [TS.rsi]
	mov  rdi, [TS.rdi]
	mov  r8,  [TS.r8]
	mov  r9,  [TS.r9]
	mov  r10, [TS.r10]
	mov  r11, [TS.r11]
	mov  r12, [TS.r12]
	mov  r13, [TS.r13]
	mov  r14, [TS.r14]
	mov  ds,  [TS.ds]
	mov  rax, [TS.rax]
	mov  rbx, [TS.rbx]
	mov  r15, [TS.r15]
	ret

;=================================================================
; Create a new kernel task.
; RAX = Code Address
; Affects RAX, RBX
;=================================================================
CreateKernelTask:
	cli
	mov  rbx, tempstack - 5*8
	mov  r15, [nextfreetss]
	mov  [TS.rsp], rbx
	mov  qword [rbx], rax
	mov  qword [rbx + 8], code64
	mov  qword [rbx + 0x10], 0x2202
	mov  qword [rbx + 0x18], tempstack0
	mov  qword [rbx + 0x20], data64
	call VCreatePageDir
	mov  [TS.waiting], 0		; Make task runnable
	mov  [TS.cr3], rax		; Fill in a few fields in the tss
	mov  [TS.ds], data64
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
	;SWITCH_TASKS
	mov  rdi, TempUserData
	mov  qword [rdi], 0
	mov  qword [rdi + 8], 0xFFE
	retq

	section '.data'

	public currentTask
	public nextfreetss

currentTask	dq 0
nextfreetss	dq TS.size
TSS64	dd 0
	dd tempstack0
	dd 0x2C dup (0)

tsslength = 0x80

