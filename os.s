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

# Final preparations before starting tasking
	mov %cr3, %rax
	mov %rax, initialCR3
	call InitMem64

	mov currentTask, %r15
	movq %r15, TS.r15(%r15)
	movb $0, TS.waiting(%r15)          	# We don't want task1 to be waiting when it starts
	movq $UserData, TS.firstfreemem(%r15)
	movq $2, TS.pid(%r15)
	movq $cd, TS.currentDirName(%r15)
	movq $0, TS.argv(%r15)
	mov  %cr3, %rax
	mov  %rax, TS.cr3(%r15)
	mov	%r15, currentTask

	mov $2, %rdi
	call AllocPage                     	# Page for kernel stack
	mov %rax, %rdi
	mov $KernelStack, %rsi
	mov $2, %rdx
	mov $3, %rcx

	call CreatePTE
	movq $KernelStack + 0x1000, %rax
	mov %rax, TSS64 + 36               	# Kernel stack pointer in TSS
	mov $2, %rdi

	call AllocPage                     	# Page for user stack
	mov %rax, %rdi
	mov $UserStack, %rsi
	mov $2, %rdx
	mov $7, %rcx
	call CreatePTE

	mov $UserStack + 0x1000, %rsp
	mov $2, %rdi
	call AllocPage                  	# Page for task code

	mov %rax, %rdi
	mov $UserCode, %rsi
	mov $2, %rdx
	mov $7, %rcx
	call CreatePTE

	mov $2, %rdi
	call AllocPage                    	# Page for task data
	mov %rax, %rdi
	mov $UserData, %rsi
	mov $2, %rdx
	mov $7, %rcx
	call CreatePTE

	mov $tas1, %rsi                   	# Move the task code
	mov $UserCode, %rdi
	mov $0x1000, %rcx                 	# How do we find the length of tas1? It's so small
	cld                               	# that we just assume it's under 0x1000 bytes
	rep movsb

	call enumeratePCIBus

	call StartTasks
	call gettime						# Set the internal clock from the RTC
	#call setclock						# Set the unixtime counter

	mov $UserCode, %rcx					# Tas1
	pushfq
	pop %r11
    or $0x200, %r11						# This will enable interrupts when we sysret

    sysretq								# Start Task1 and multitasking

	.data
	
cd:	.byte '/', 0

	.global tempstack

gdt_48:	.word 0x800						# Allow up to 512 entries in GDT
	.long GDT

idt_64:	.word 0x800						# Allow up to 512 entries in IDT
	.quad IDT

# A minimal stack whilst the system is being initialized
.rept 128
	.quad 0
.endr
tempstack:
