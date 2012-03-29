.include "include/memory.inc"
OsCodeSeg 	= 0x8
OsDataSeg 	= 0x10
code64	  	= 0x18

#=========================================================================
# This file contains the code to switch the processor to 64-bit long mode
#=========================================================================

	.text

	.global _start

	.code32

#=====================
# Protected Mode code
#=====================
_start:
here:
	mov $OsDataSeg, %eax
	mov %eax, %ds
	mov %eax, %ss
	mov $tempstack, %esp
	call InitMemManagement
# Zero unused pages
	mov nPages, %edx
	mov $0, %ebx
	mov $PageMap, %eax
again:
	movw (%eax), %bx
	cmpw $0, %bx
	jne nextpage
	mov %eax, %edi
	sub $PageMap,%eax
	sal $11,%eax 
	mov $1000, %ecx
again2:
	movb $0, (%eax)
	add $1, %eax
	sub $1, %ecx
	cmp $0, %ecx
	jne again2
	mov %edi, %eax
nextpage:
	sub $1, %edx
	cmp $0, %edx
	je  finished
	add $2, %eax
	jmp again
finished:
	call HwSetup

	# create 64-bit page tables
	call CreatePageDir             # task1
	mov %eax, %cr3

	mov %cr4, %eax
	bts $5, %eax                   # Physical Address extension bit
	bts $7, %eax                   # Enable global pages
	mov %eax, %cr4

	mov $0xc0000080, %ecx          # These 2 instructions read the Extended Feature
	rdmsr                          # Enable register
	bts $0, %eax                   # This enables the syscall/sysret instructions
	bts $8, %eax                   # bit 8 of this register is Long Mode Enable (but we won't
	wrmsr                          # actually be in Long Mode until paging is enabled)

	mov	%cr0, %eax
	bts $16, %eax                  # Enable paging
	bts $31, %eax                  # Enable write protection
	mov %eax, %cr0

	jmp $code64, $start64
