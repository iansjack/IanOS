include 'macros.asm'
include 'memory.h'

;=========================================================================
; This file contains the code to switch the processor to 64-bit long mode
;=========================================================================
	format ELF

	section '.text' executable

	public _start

	extrn InitMemManagement
	extrn hwsetup
	extrn CreatePageDir
	extrn start64
	extrn tempstack


;=====================
; Protected Mode code
;=====================
_start:	
here:
	mov ax,OsDataSeg       
	mov ds,ax
	mov ss,ax
	call InitMemManagement
	call hwsetup
	mov esp, tempstack

	; create 64-bit page tables
	call CreatePageDir	    	; task1
	mov  cr3, eax

	mov  eax, cr4
	bts  eax, 5			; Physical Address extension bit
	mov  cr4, eax

	mov  ecx, 0xc0000080		; These 2 instructions read the Extended Feature
	rdmsr				; Enable register
	bts  eax, 0			; This enables the syscall/sysret instructions
	bts  eax, 8			; bit 8 of this register is Long Mode Enable (but we won't
	wrmsr				; actually be in Long Mode until paging is enabled)

	mov  eax, cr0			; enable paging
	bts  eax, 31
	mov  cr0, eax

	jmp  code64:start64
