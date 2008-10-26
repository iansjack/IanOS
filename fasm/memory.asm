	section '.text' executable

AllocPage64:
	push rbx
	; find first free page of Physical Memory
	mov rbx, 0x0
.again: cmp byte [PageMap + rbx], 0
	je .found
	inc rbx
	jmp .again
	; mark page as in use
.found: mov byte [PageMap + rbx], al
	; compute Physical Address
	xchg rax, rbx
	shl rax, 12
	dec word [nPagesFree]
	pop rbx
	ret

;==============================================================================
; Searches the linked list pointed to by RBX for a block of memory of size RAX
; Allocates the memory and returns its address in RAX
;==============================================================================
AllocMem:
	push rbx
	push rcx
	push rdx
	add  rax, 0x10		; Need another 16 bytes to accommodate the pointers tracking memory use
.next:	cmp  [rbx + 8], rax	; [RBX + 8] = size of this free block of memory (or 0 if it's already allocated)
	jge  .found
	cmp  qword [rbx], 0
	je   .nofreemem
	mov  rbx, [rbx] 	; This block isn't big enough so get the pointer to to next one
	jmp  .next
.nofreemem:
	mov  rax, 0		; No memory block big enough available, so return 0
	ret
.found: mov  rcx, [rbx] 	; We've found a block of memory at least as large as that requested (+ 16)
	mov  rdx, rbx
	add  rdx, rax
	mov  [rdx], rcx
	mov  [rbx], rdx
	mov  rcx, [rbx + 8]
	mov  qword [rbx + 8], 0
	mov  [rdx + 8], rcx
	sub  [rdx + 8], rax
	mov  rax, rbx
	add  rax, 16
	pop  rdx
	pop  rcx
	pop  rbx
	ret

;==================================================
; Deallocate the memory at location RAX.
; This will deallocate both user and kernel memory
;==================================================
DeallocMem:
	push rbx
	mov  rbx, [rax - 0x10]
	sub  rbx, rax
	mov  [rax - 0x8], rbx
	pop  rbx
	ret

;=====================================================================
; Allocate some kernel memory from the heap. RAX = amount to allocate
; Returns in RAX address of allocated memory.
; Uses a mem structure
; nextmem dq ?  pointer to next mem structure
; size    dq ?  size of this mem structure (including these two quads)
;======================================================================
AllocKMem:
	push rbx
.tryAgain:
	push rax
	mov  rbx, [firstFreeKMem]
	call AllocMem
	cmp  rax, 0
	jne  .done		 ; An appropriate memory block was found and allocated so return it in RAX
	call AllocPage64	 ; No big enough block of memory was found, so add another page to the Kernel heap
	mov  rbx, [nextKPage]
	shl  rbx, 12
	call CreatePTE
	inc  qword [nextKPage]
	mov  rbx, [firstFreeKMem]
.l2:	cmp  qword [rbx], 0
	je   .l1
	mov  rbx, [rbx]
	jmp  .l2
.l1:	add  qword [rbx + 8], 0x1000   ; Add the extra page into the amount of free space
	pop  rax
	jmp  .tryAgain
.done:	pop  rbx		       ; Throw away the RAX that we pushed after .tryAgain
	pop  rbx
	ret

	section '.data'

	public oMemMax
	public nPagesFree

oMemMax    dd 0
nPagesFree dd 0
