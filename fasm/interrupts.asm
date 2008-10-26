SLEEPINT = 2
KBDINT = 1
HDINT = 14

	section '.text' executable

	public WaitForInt
	public Ps

;================
; Timer interrupt
;================
TimerInt:
	push rax
	mov  al, 0x20
	out  0x20, al
	inc  [Ticks]
	cmp  [Timer.active], 0
	je   .notimer
	dec  [Timer.interval]
	jnz  .notimer
	mov  r15, [Timer.task]
	mov  [TS.waiting], 0
	mov  [Timer.active], 0
.notimer:
	pop  rax
	dec  [TimeSliceCount]
	jnz  .done
	mov  [TimeSliceCount], 5
	SWITCH_TASKS
.done:	iretq

;=====================
; Hard Disk interrupt
;=====================
HdInt:	push rax
.istaskwaiting:
	mov  r15, [currentTask]
	mov  al, 0x20
	out  0x20, al
	out  0xA0, al
.again: cmp  [TS.waiting], HDINT
	jne  .goon
	mov  [TS.waiting], 0
	SWITCH_TASKS_R15
	jmp  .done
.goon:	mov  r15, [TS.nexttask]
	cmp  r15, [currentTask]
	jne  .again
.done:	pop  rax
	iretq

;===========================================================================================================
; We need to wrap these two subroutines inside interrupt routines.
; Thus we can call them from, e.g., a SYSCALL. (They expect to be called from within an interrupt routine.)
;===========================================================================================================
SwitchTasks:			; int 20
	cli
	call TaskSwitch
	iretq

SpecificSwitchTasks:		; int 22
	cli
	call SpecificTaskSwitch
	iretq

intr:	mov al,0x20	 ; clear int
	out 0x20,al
	iretq

div0:	WRITE_STRING div0message, 0, 0
	WRITE_DOUBLE [esp], 0, 60
	WRITE_DOUBLE [esp + 4], 1, 60
	iretq
i1:	mov byte [0xB8000], '1'
	iretq
i2:	mov byte [0xB8000], '2'
	iretq
i3:	mov byte [0xB8000], '3'
	iretq
i4:	mov byte [0xB8000], '4'
	iretq
i5:	mov byte [0xB8000], '5'
	iretq
i6:	mov byte [0xB8000], '6'
	iretq
i7:	mov byte [0xB8000], '7'
	iretq
i8:	mov byte [0xB8000], '8'
	iretq
i9:	mov byte [0xB8000], '9'
	iretq
ia:	mov byte [0xB8000], 'a'
	iretq
ib:	mov byte [0xB8000], 'b'
	iretq
ic:	mov byte [0xB8000], 'c'
	iretq
gpf:	KWRITE_STRING GPFmessage, 0, 0
	KWRITE_DOUBLE [esp+0x20], 0, 60
	KWRITE_DOUBLE [esp+0x28], 1, 60
	KWRITE_DOUBLE [esp+0x30], 2, 60
	KWRITE_DOUBLE [esp+0x38], 3, 60
	KWRITE_DOUBLE [esp+0x40], 4, 60
	KWRITE_DOUBLE [esp+0x48], 5, 60
	pop rax
	hlt
	iretq
pf:	KWRITE_STRING PFmessage, 0, 0
	KWRITE_DOUBLE [esp+0x20], 0, 60
	KWRITE_DOUBLE [esp+0x28], 1, 60
	KWRITE_DOUBLE [esp+0x30], 2, 60
	KWRITE_DOUBLE [esp+0x38], 3, 60
	KWRITE_DOUBLE [esp+0x40], 4, 60
	KWRITE_DOUBLE [esp+0x48], 5, 60
	pop rax
	hlt
	iretq
itf:	mov byte [0xB8000], 'f'
	iretq
ig:	mov byte [0xB8000], 'g'
	iretq

Pd:	mov ax, 160
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
	mov [ebx + 0xB8000], al
	add bx,2
	loop .stillCounting
	ret

Ps:	mov ax, 160
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
.done:	ret

HD_PORT=0x1F0
int21:	mov dx, HD_PORT+7
	push rax
.again2:
	in  al, dx
	test al, 0x80
	jnz .again2
	mov dx, HD_PORT+2     ; 0x1F2 - sector count
	mov al, 1
	out dx, al
	inc dx		      ; 0x1F3
	pop rax
	mov ebx, eax
	and al, 0xFF
	out dx, al	      ; lba lo
	inc dx		      ; 0x1F4
	mov eax, ebx
	and ah, 0xFF
	mov al, ah
	out dx, al	      ; lba mid
	inc dx		      ; 0x1F5
	mov eax, ebx
	shr eax, 16
	and al, 0xFF
	out dx, al	      ; lba hi
	inc dx		      ; 0x1F6
	and ah, 0xF
	mov al, ah
	add al, 0x40	      ; lba mode/drive /lba top
	out dx, al
	inc dx		      ; 0x1F7
	mov ax, 0x20
	out dx, al
	WAIT_FOR_INT HDINT
.again3:
	in  al, dx
	test al,0x80
	jnz .again3
	mov dx, HD_PORT
	mov eax, 0
	mov rcx, 256
	mov rdi, DiskBuffer
	cld
	rep
	insw
	iretq

;==============================================================================
; Stop the current task and make it wait for the interrupt number passed in AL
;==============================================================================
WaitForInt:
	mov  r15, [currentTask]
	mov  [TS.waiting], al
	SWITCH_TASKS		       ; The current task is no longer runnable
	ret

;===========================================
; Initialize the Interrupt Descriptor Table
;===========================================
InitIDT:
	mov rax, code64
	mov rbx, intr
	mov rdx, 0
	mov rcx, 0
.again:	push rcx
	call CreateTrapGate
	pop rcx
	add rcx, 0x10
	inc rdx
	cmp rdx, 48
	jne .again
	mov rbx, gpf
	mov rcx, 13 * 0x10
	call CreateTrapGate
	mov rbx, pf
	mov rcx, 14 * 0x10
	call CreateTrapGate
	mov rbx, SwitchTasks
	mov rcx, 20 * 0x10
	call CreateTrapGate
	mov rbx, int21
	mov rcx, 21 * 0x10
	call CreateTrapGate
	mov rbx, SpecificSwitchTasks
	mov rcx, 22 * 0x10
	call CreateTrapGate
	mov rbx, TimerInt
	mov rcx, 32 * 0x10
	call CreateIntGate
	mov rbx, KbInt
	mov rcx, 33 * 0x10
	call CreateIntGate
	mov rbx, HdInt
	mov rcx, 46 * 0x10
	call CreateIntGate
	ret

	section '.data'

	public Ticks
	public Timer.active
	public Timer.interval
	public Timer.task

Ticks	       dq 0
TimeSliceCount db 5
Timer.active 	db 0
Timer.interval	dq 0
Timer.task		dq 0
