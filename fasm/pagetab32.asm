include 'memory.h'

	format ELF

	public CreatePageDir

	extrn AllocPage

	section '.text' executable

;================================================================================
; Create a Page Directory with the necessary entries in the first Page Table
; return the Physical Address of this Page Directory in eax
; This only works in 32 bit mode before paging is enabled
;================================================================================
CreatePageDir:
	mov eax, 0xFF
	call AllocPage	; for level 4 table
	push eax
	mov  ebx, eax
	mov eax, 0xFF
	call AllocPage	; for level 3 table
	mov  [ebx], eax
	mov  dword [ebx + 4], 0
	add  dword [ebx], 0x7
	mov  ebx, eax
	mov eax, 0xFF
	call AllocPage	; for level 2 table
	mov  [ebx], eax
	mov  dword [ebx + 4], 0
	add  dword [ebx], 0x7
	mov  ebx, eax
	mov eax, 0xFF
	call AllocPage	; for level 1 table 1     ; 0x000000 - 0x1FFFFF (OS & stacks)
	mov  [ebx], eax
	mov  dword [ebx + 4], 0
	add  dword [ebx], 0x7
	push ebx
	call CreatePT164
	pop ebx
	mov eax, 0xFF
	call AllocPage	; for level 1 table 2     ; 0x200000 - 0x3FFFFF (Page Tables)
	mov [ebx + 8], eax
	mov dword [ebx + 12], 0
	add dword [ebx + 8], 7
	mov ebx, eax
	pop eax
	push eax
	call CreatePT264
	pop eax
	ret

;=====================================================================
; Create the OS Page Table referred to by the above Page Directory
; This does no remapping - each Logical Adress is mapped to the same
; Physical Address. This covers Physical Addresses from 0 to 0x200000.
; EAX = Physical Address of Page Table
;=====================================================================
CreatePT164:
	mov esi, eax
	mov ecx, 0x200
	mov ebx, 0
	mov eax, 0x1
	mov edx, PageMap
again:	mov dword [ebx + esi], 0
	mov dword [ebx + esi + 4], 0
	cmp byte [edx], 0
	je  .notused
	cmp byte [edx], 0xFF
	je  .notused			; well - used by task 1, but we don't want to map it
	mov [ebx + esi], eax
.notused:
	inc edx
	add ebx,8
	add eax, 0x1000
.notdata:
	loop again
	ret

;==========================================
; Create the Page Table for the Page Tables
; EAX = PA of level 4 table
; EBX = PA of level 1 table 2
;==========================================
CreatePT264:
	mov dword [ebx], eax	 ; level 4
	add dword [ebx], 7
	mov dword [ebx + 4], 0
	mov eax, [eax]
	and eax, 0xFFFFF000
	mov dword [ebx + 8], eax ; level 3
	add dword [ebx + 8], 7
	mov dword [ebx + 12], 0
	mov eax, [eax]
	and eax, 0xFFFFF000
	mov dword [ebx + 16], eax ; level 2
	add dword [ebx + 16], 7
	mov dword [ebx + 20], 0
	push eax
	mov eax, [eax]
	and eax, 0xFFFFF000
	mov dword [ebx + 24], eax ; level 1 table 1
	add dword [ebx + 24], 7
	mov dword [ebx + 28], 0
	pop eax
	mov eax, [eax + 8]
	and eax, 0xFFFFF000
	mov dword [ebx + 32], eax ; level 1 table 2
	add dword [ebx + 32], 7
	mov dword [ebx + 36], 0
	ret

