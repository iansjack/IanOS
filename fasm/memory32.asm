include 'memory.h'

	format ELF
	
	public InitMemManagement
	public AllocPage

	extrn nPagesFree
	extrn oMemMax

	section '.text' executable

;==================================
; Find top of memory
;==================================
InitMemManagement:
	mov dword [nPagesFree], 256
	mov eax, 0x0019FFFC
	mov ebx, 0
	mov ecx, 0x6d72646c
MemLoop:
	mov dword [eax], ecx
	mov ebx, dword [eax]
	cmp ebx, ecx
	jne MemLoopEnd
	add eax, 0x00060003
	mov [oMemMax], eax
	sub eax, 0x00060003
	add eax, 0x00100000
	add dword [nPagesFree], 256
	mov ebx, 0
	jmp MemLoop
MemLoopEnd:
	mov ecx, 0x00004000
.a:	mov byte [PageMap + ecx], 0
	loop .a
	; Mark used memory in MemoryMap
	; all this is defined as belonging to Task 1

	; GDT and IDT
	mov byte [PageMap], 1
	dec dword [nPagesFree]

	; OS Memory
	; mov ecx, DataSegLen
	mov ecx, 0x1000
	shr ecx, 0xB
	add ecx, 2		  ; # of pages in OS data segment
	add ecx, 0xF		  ; # of pages in OS code segment
	mov ebx, PageMap
.again: mov byte [ebx], 1
	inc ebx
	dec dword [nPagesFree]
	loop .again

	; 0x0006F000 - 0x0006FFFF Static Message Ports
	mov byte [PageMap + 0x6F], 1

	; 0x00070000 - 0x00071000 Disk Buffer
	mov byte [PageMap + 0x70], 1

	; 0x0007F000 - 0x0007FFFF Page Map
	mov byte [PageMap + 0x7F], 1

	; 0x00080000 - 0x00081000, TaskStructs
	mov byte [PageMap + 0x80], 1

	; 0x000A0000 - 0x00100000
	mov ebx, PageMap + 0xA0
	mov ecx, 0x60
.ag2:	mov byte [ebx], 1
	inc ebx
	dec dword [nPagesFree]
	loop .ag2
	ret

;===========================================
; Allocate a page of memory
; AL = task to allocate memory to
; if EAX = 0xFF then memory is for PT
; return in EAX the Physical Memory address
;===========================================
AllocPage:
	push ebx
	; find first free page of Physical Memory
	mov ebx, 0x0
.again: cmp byte [PageMap + ebx], 0
	je .found
	inc ebx
	jmp .again
	; mark page as in use
.found: mov byte [PageMap + ebx], al
	; compute Physical Address
	xchg eax, ebx
	shl eax, 12
	dec dword [nPagesFree]
	pop ebx
	ret
