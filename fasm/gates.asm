	section '.text' executable

;=======================
; Create a Trap Gate
; RAX = selector
; RBX = offset
; RCX = interrupt #
;=======================
CreateTrapGate:
	add rcx, IDT
	mov word [rcx], bx
	mov word [rcx + 2], ax
	mov byte [rcx + 4], 0
	mov byte [rcx + 5], 0x8F
	mov word [rcx + 6], 0
	mov qword [rcx + 8], 0
	ret

;==========================
; Create an Interrupt Gate
; RAX = selector
; RBX = offset
; RCX = interrupt #
;==========================
CreateIntGate:
	add rcx, IDT
	mov word [rcx], bx
	mov word [rcx + 2], ax
	mov byte [rcx + 4], 0
	mov byte [rcx + 5], 0xEE
	mov word [rcx + 6], 0
	mov qword [rcx + 8], 0
	ret

tsslength = 0x80

;=========================
; Create a TSS descriptor
; RAX = base
; RCX = selector #
;=========================
CreateTssDesc:
	add rcx, GDT
	mov word [rcx], tsslength
	and word [rcx], 0xFFFF
	mov rbx, rax
	and rbx, 0xFFFF
	mov word [rcx + 2], bx
	mov rbx, rax
	shr rbx, 16
	and rbx, 0xFF
	mov byte [rcx + 4], bl
	mov byte [rcx + 5], 0x89
	mov byte [rcx + 6], 0
	mov rbx, rax
	shr rbx, 24
	and rbx, 0xFF
	mov byte [rcx + 7], bl
	mov qword [rcx + 8], 0
	ret


	