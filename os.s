	.include "include/memory.inc"
	.include "include/syscalls.inc"
	.include "include/kstructs.inc"

	.text

	.global start64

#======================================
# From here on we are using 64 bit code
#======================================
start64:
	mov $data64, %ax
	mov %ax, %ds
	mov %ax, %ss
	mov $0xC0000081, %ecx
	mov $0x00230018, %edx
	mov $SysCalls, %eax
	wrmsr
	mov $0xC0000082, %ecx
	mov $0, %edx
	mov $SysCalls, %eax
	wrmsr
	mov $0xC0000083, %ecx
	mov $0, %edx
	mov $0, %eax
	wrmsr
	mov $0xC0000084, %ecx
	mov $0, %edx
	mov $SysCalls, %eax
	wrmsr
	call InitIDT
	mov $TSS64, %rdi
	mov $tssd64, %rsi
	call CreateTssDesc
	mov $tssd64, %ax
	ltr %ax
	lidt idt_64

	mov  	%cr0, %rax
	bts	$16, %rax					# enable write protection
#	bts  	$31, %eax					# enable paging
	mov  %rax, %cr0

# Final preparations before starting tasking
	call InitMem64
	mov $0xFFF, %rcx			# Zero page of memory locations for task structures
	mov $0, %rax
	mov $0x200, %rcx
	mov $TaskStruct, %rdi
	cld
	rep stosq
	
	mov $TaskStruct, %r15			# Set up skeleton task structure for first task
	movq $0, TS.nexttask(%r15)
	movq %r15, TS.r15(%r15)
	movb $0, TS.waiting(%r15)		# We don't want task1 to be waiting when it starts
	movq $UserData, TS.firstfreemem(%r15)
	movq $2, TS.pid(%r15)
	movq $0, TS.currentDir(%r15)
	mov  %cr3, %rax
	mov  %rax, TS.cr3(%r15)
	mov	%r15, currentTask

	mov $0xFF, %al
	call AllocPage					# Page for kernel stack
	mov %rax, %rdi
	mov $KernelStack, %rsi
	call CreatePTE
	mov $KernelStack + 0x1000, %eax
	mov %eax, TSS64 + 4			# Kernel stack pointer in TSS
	mov $0xFF, %al
	call AllocPage					# Page for user stack
	mov %rax, %rdi
	mov $UserStack, %rsi
	call CreatePTE
	mov $UserStack + 0x1000, %rsp
	mov $0xFF, %al
	call AllocPage					# Page for task code
	mov %rax, %rdi
	mov $UserCode, %rsi
	call CreatePTE
	mov $0xFF, %al
	call AllocPage		  		# Page for task data
	mov %rax, %rdi
	mov $UserData, %rsi
	call CreatePTE

	mov $tas1, %rsi				# Move the task code
	mov $UserCode, %rdi
	mov $0x1000, %rcx				# How do we find the length of tas1? It's so small
	cld								# that we just assume it's under 0x1000 bytes
	rep movsb

	call StartTasks

	mov $UserCode, %rcx		  	# Task 1
	pushfq
	pop %r11
	or $0x200, %r11		  		# This will enable interrupts when we sysret

	sysretq				  			# Start Task1 and multitasking

	.data

	.global tempstack

gdt_48: .word	0x800				# Allow up to 512 entries in GDT
	.long	GDT

idt_64: .word 0x800				# Allow up to 512 entries in IDT
	.quad IDT

# A minimal stack whilst the system is being initialized
.rept 128
	.quad 0
.endr
tempstack:
