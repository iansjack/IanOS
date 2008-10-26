macro REX
{
	db 0x48
}

macro sysret
{
	REX
	sysret
}

macro SYS callnum
{
	mov r9, callnum
	syscall
}

macro CALL_GATE64 name,selector,offset
{
	name = $ - mygdt
	dw	offset + OSCode
	dw	selector
	db	0
	db	0xEC
	dw	0
	dq	0
}

macro INTR_GATE64 selector,offset
{
	dw	offset + OSCode
	dw	selector
	db	0
	db	0xEE
	dw	0
	dq	0
}

macro TRAP_GATE64 selector,offset
{
	dw	offset + OSCode
	dw	selector
	db	0
	db	0x8F
	dw	0
	dq	0
}

macro SEG_DESCRIP name,base,g,segsize,longmode,limit,p,dpl,dt,type
{
	name = $ - mygdt
	dw	limit and 0xffff
	dw	base and 0xffff
	db	(base and 0xff0000) shr 16
	db	(p shl 3 + dpl shl 1 + dt) shl 4 + type
	db	(g shl 3 + segsize shl 2 + longmode shl 1) shl 4 + ((limit and 0xf0000) shr 16)
	db	(base and 0xff000000) shr 24
}

macro CODE_SEG_DESCR name,base,limit,dpl
{
	SEG_DESCRIP name,base,1,1,0,limit,1,dpl,1,8
}

macro CODE64_SEG_DESCR name,dpl
{
	SEG_DESCRIP name, 0, 0, 0, 1, 0, 1, dpl, 1, 8
}	

macro DATA_SEG_DESCR name,base,limit
{
	SEG_DESCRIP name,base,1,1,0,limit,1,0,1,2
}

macro DATA64_SEG_DESCR name,dpl
{
	SEG_DESCRIP name,0,0,0,1,0,1,dpl,1,2
}

macro TSS_SEG_DESCR64 name,base
{
	SEG_DESCRIP name,base,0,0,0,tsslength,1,0,0,9
	dq 0
}

macro WRITE_CHAR char, row, col
{
	push rax
	push rbx
	mov ah, char
	mov ebx, 0
	mov bh, row
	mov bl, col
	SYS PRINTCHAR
	pop rbx
	pop rax
}

macro WRITE_DOUBLE number, row, col
{
	push rax
	push rbx
	push rcx
	push rdx
	push r9
	mov edx, number
	;mov al, 1
	mov ebx, 0
	mov bh, row
	mov bl, col
	SYS PRINTDOUBLE
	pop r9
	pop rdx
	pop rcx
	pop rbx
	pop rax
}

macro WRITE_STRING str, row, col
{
	push rax
	push rbx
	push rdx
	push r9
	mov edx, str
	mov ebx, 0
	mov bh, row
	mov bl, col
	SYS PRINTSTRING
	pop r9
	pop rdx
	pop rbx
	pop rax
}

macro KWRITE_DOUBLE number, row, col
{
	push rax
	push rbx
	push rcx
	push rdx
	mov edx, number
	mov al, 1
	mov ebx, 0
	mov bh, row
	mov bl, col
	call Pd
	pop rdx
	pop rcx
	pop rbx
	pop rax
}

macro KWRITE_STRING str, row, col
{
	push rax
	push rbx
	push rdx
	mov edx, str
	mov ebx, 0
	mov bh, row
	mov bl, col
	call Ps
	pop rdx
	pop rbx
	pop rax
}

macro GET_KEY
{
	push rbx
	SYS GETKEY
	pop rbx
}
macro SETUP_STACKS taskno
{
	mov rbx, KernelStack		  ; Every task must start like this
	mov al, 0xFF
	SYS ALLOCPAGE			  ;
	mov rbx, UserStack		  ;
	mov al, 0xFF
	SYS ALLOCPAGE			  ;
	mov rsp, UserStack + 0x1000	  ;
}

macro KSETUP_STACKS
{
	mov rbx, KernelStack		  ; Every kernel task must start like this
	mov al, 0xFF
	call AllocPage64
	call CreatePTE			  ; map this page to [RBX]
	mov rbx, UserStack		  ;
	mov al, 0xFF
	call AllocPage64
	call CreatePTE			  ; map this page to [RBX]
	mov rsp, UserStack + 0x1000	  ;
}

macro WAIT_FOR_INT intno
{
	mov al, intno
	call WaitForInt
}

macro SWITCH_TASKS
{
	int 20
}

macro SWITCH_TASKS_R15
{
	int 22
}

macro READ_SECTOR
{
	int 21
}