	section '.text' executable

;=====================================
; Read in some parameters from the HD
;=====================================
InitializeHD:
	mov eax, 0				  	; Read partition table
	READ_SECTOR
	mov eax, [dword DiskBuffer + 0x1BE + 0x8]	; Read boot sector
	push rax
	READ_SECTOR
	mov al, [dword DiskBuffer + 0x0D]		; Sectors per cluster
	mov [SectorsPerCluster], al
	mov eax, [dword DiskBuffer + 0x16]		; Sectors per FAT
	shl eax, 1				  	; Two FATs
	add eax, [dword DiskBuffer + 0x0E]		; Add on number of reserved sectors
	pop rbx
	add eax, ebx				  	; Add on offset to boot sector
	and eax, 0xFFFF 			  	; Mask out unwanted bits. AX now contains location of root dir
	mov [RootDir], eax
	mov eax, 0
	mov ax, [dword DiskBuffer + 0x11]		; # of root entries
	shr ax, 4
	add eax, [RootDir]
	mov [DataStart], eax
	ret

;============================================================================
; Find the root directory entry for the file whose name is pointed to by RSI
; Return the address of the directory entry in RDX
; Affects RBX, RCX, RDX, RDI, RSI
;============================================================================
FindFile:
	mov rbx, rsi
	mov rdx, DiskBuffer
.again: mov rdi, rdx
	mov rsi, rbx
	mov rcx, 11
	repe
	cmpsb
	cmp rcx, 0
	je  .found
	add rdx, 0x20
	jmp .again
.found: ret

;==============================================================================
; Find the first sector of the file whose directory entry is pointed to by RDX
; Return sector number in RAX
;==============================================================================
FindFirstSect:
	mov  eax, 0
	mov  ax, [rdx + 0x1A]
	dec  ax
	dec  ax
	mov  ebx, 0
	mov  bl, [SectorsPerCluster]
	imul eax, ebx				; First sector of above file
	add  eax, [DataStart]
	ret

;===============================================
; Load the file whose name is pointed to by RSI
;===============================================
LoadFile:
	push rbx
	push rcx
	push rdx
	mov  eax, [RootDir]
	READ_SECTOR
	call FindFile
	call FindFirstSect
	READ_SECTOR
	pop  rdx
	pop  rcx
	pop  rbx
	ret

	section '.data'

RootDir 	  dd 0
DataStart	  dd 0
SectorsPerCluster db 0
