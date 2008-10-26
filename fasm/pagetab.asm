	section '.text' executable

	public CreatePTE

;======================================================================================
; A version of the above routine to work when paging is enabled.
; Affects RAX, RBX, RSI
;======================================================================================
VCreatePageDir:
	mov  al, 0xFF
	call AllocPage64		 ; page for new PageTableL4
	push rax
	mov rbx, TempPTL4
	call CreatePTE
	mov  al, 0xFF
	call AllocPage64		 ; page for new PageTableL3
	mov rbx, TempPTL3
	call CreatePTE
	mov [qword TempPTL4], rax
	mov  al, 0xFF
	call AllocPage64		 ; page for new PageTableL2
	mov rbx, TempPTL2
	call CreatePTE
	mov [qword TempPTL3], rax
; Set up PT1
	mov  rax, [qword PageTableL12 + 0x18]  ; OS Page 1 - all Page Directories share this page
	mov  [qword TempPTL2], rax
	mov  al, 0xFF
	call AllocPage64		 ; page for new PageTableL12
	mov rbx, TempPTL12
	call CreatePTE
	mov [qword TempPTL2 + 8], rax
; Create new PT2
	mov rsi, TempPTL12
	pop rax
	push rax
	add rax, 7
	mov [rsi], rax
	mov rax, [qword TempPTL4]
	mov [rsi + 8], rax
	mov rax, [qword TempPTL3]
	mov [rsi + 16], rax
	mov rax, [qword TempPTL2]
	mov [rsi + 24], rax
	mov rax, [qword TempPTL2 + 8]
	mov [rsi + 32], rax
	mov al, 0xFF
	call AllocPage64		; page for task code
	mov rbx, TempUserCode
	call CreatePTE
	mov [qword TempPTL12 + 0x800], rax	; 0x800 = UserCode shr 9 and 0xFFF
	mov al, 0xFF
	call AllocPage64		; page for task data
	mov rbx, TempUserData
	call CreatePTE
	mov [qword TempPTL12 + 0x880], rax	; 0x880 = UserData shr 9 and 0xFFF
	pop rax
	ret

;============================================
; Create a Page Table Entry
; RAX = Physical Address
; RBX = Logical Address
; Affects RAX, RBX
;============================================
CreatePTE:
	shr rbx, 12		     ; shr 12 to mask off offset - rbx = PTL4|PTL3|PTL2|PTL1
	shl rbx, 3		     ; sl3 cause each entry is 8 bytes
	add rbx, PageTableL11	     ; rbx now points to the appropriate
	or  rax, 7
	mov [rbx], rax
	ret

