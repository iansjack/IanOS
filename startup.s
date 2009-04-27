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
	call InitMemManagement
	call HwSetup
	mov $tempstack, %esp

	# create 64-bit page tables
	call CreatePageDir	    		# task1
	mov  %eax, %cr3

	mov  %cr4, %eax
	bts  $5, %eax				# Physical Address extension bit
	mov  %eax, %cr4

	mov  $0xc0000080, %ecx			# These 2 instructions read the Extended Feature
	rdmsr					# Enable register
	bts  $0, %eax				# This enables the syscall/sysret instructions
	bts  $8, %eax				# bit 8 of this register is Long Mode Enable (but we won't
	wrmsr					# actually be in Long Mode until paging is enabled)

	mov  %cr0, %eax				# enable paging
	bts  $31, %eax
	mov  %eax, %cr0

	jmp  $code64, $start64
