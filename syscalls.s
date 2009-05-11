	SLEEPINT = 2

	.include "memory.h"
	.include "kstructs.h"
	.include "macros.s"

	.text

	.global SysCalls

CallNo:	
	.quad	AllocatePage64		# ALLOCPAGE
	.quad	PrintString		# PRINTSTRING
	.quad	PrintDouble		# PRINTDOUBLE
	.quad	PrintChar		# PRINTCHAR
	.quad	Newtask 		# CREATETASK
	.quad	ClearScreen		# CLEARSCREEN
	.quad	GetTicks		# GETTICKS
	.quad	Sleep			# SLEEP
	.quad	Alloc_Mem		# ALLOCMEM
	.quad	Alloc_Message_Port	# ALLOCMSGPORT
	.quad	Send_Message		# SENDMESSAGE
	.quad	Receive_Message 	# RECEIVEMESSAGE
	.quad	Dealloc_Mem		# DEALLOCMEM
	.quad	Send_Receive		# SENDRECEIVE
	.quad	Kill_Task		# KILLTASK
	.quad	Halt			# HALT
	.quad   NewKerneltask		# CREATEKTASK
	.quad	Alloc_Shared_Mem	# ALLOCSHAREDMEM
	.quad	GetCR3			# GETCR3
	.quad	NewLPtask		# CREATELPTASK

SysCalls:
	jmp *(CallNo - 8)(,%r9, 8)

#===========================================
# Allocate a page of memory (64-bit version)
# Create a PTE for it pointing to RDI
# return in RAX the Physical Memory address
#===========================================
AllocatePage64:
	push %rcx
	call AllocPage64
	mov %rdi, %rsi
	mov %rax, %rdi
	call CreatePTE
	pop %rcx
	sysretq

#========================================================
# Print [EDX] as string at position row BH col BL
# Affects RAX, RBX, RDX
#========================================================
PrintString:
	mov $160, %ax
	mul %bh
	mov $0, %bh
	shl $1, %bx
	add %ax, %bx
.isItStringEnd:
	mov (%edx), %ah
	cmp $0, %ah
	je .done
	mov %ah, 0xB8000(%ebx)
	add $2, %bx
	inc %edx
	jmp .isItStringEnd
.done:	sysretq

#========================================================
# Print EDX as hex at position row BH col BL
# Affects RAX, RBX, RDX
#========================================================
PrintDouble:
	push %rcx
	mov $160, %ax
	mul %bh
	mov $0, %bh
	shl $1, %bx
	add %ax, %bx
	mov $8, %rcx
.stillCounting:
	shld $4, %edx, %eax
	shl $4, %edx
	and $0xF, %eax
	add $'0, %al
	cmp $'9, %al
	jle .under10
	add $7, %al
.under10:
	mov  %al, 0xB8000(%ebx)
	add  $2, %bx
	loop .stillCounting
	pop  %rcx
	sysretq

#========================================================
# Print character in AH at position row BH col BL
# Affects RAX, RBX
#========================================================
PrintChar:
	push %ax
	mov  $160, %ax
	mul  %bh
	mov  $0, %bh
	shl  $1, %bx
	add  %ax, %bx
	pop  %ax
	mov  %ah, 0xB8000(%ebx)
	sysretq

#=================================================================
# Create a new task from the file whose name is pointed to by RDI
# Affects RAX, RDI
#=================================================================
Newtask:
	push %rcx
	call NewTask
	int  $20
	pop %rcx
	sysretq

#======================
#Clear the console
#======================
ClearScreen:
	push %rbx
	push %rcx
	mov  $0xB8000, %rbx
	mov  $0xFA0, %rcx
.again: movw $0x0720, -2(%rbx, %rcx)
	dec  %rcx
	loop .again
	pop  %rcx
	pop  %rbx
	sysretq

#=============================================================================
# Return in RAX the number of (10ms) clock ticks since the system was started
#=============================================================================
GetTicks:
	mov Ticks, %rax
	sysretq

#=====================================================
# Suspend the current task for for RDI 1ms intervals
#=====================================================
Sleep:	push %rcx
	mov currentTask, %r15
	movb $SLEEPINT, TS.waiting(%r15)
	mov %rdi, TS.timer(%r15)
	mov %r15, %rdi
	call BlockTask
	SWITCH_TASKS		       # The current task is no longer runnable
	pop %rcx
	sysretq

#==============================================================
# Allocate some memory from the heap. RDI = amount to allocate
# Returns in RAX address of allocated memory.
#==============================================================
Alloc_Mem:
	push %rcx
	mov  currentTask, %r15
	mov  TS.firstfreemem(%r15), %rsi
	call AllocMem
	pop  %rcx
	sysretq

#=================================================
# Allocate a message port. Return the port in RAX
#=================================================
Alloc_Message_Port:
	push %rcx
	call AllocMessagePort
	pop %rcx
	sysretq

#===================================
# Send a message to a message port.
# RDI = message port
# RSI = message
#===================================
Send_Message:
	push %rcx
	call SendMessage
	pop %rcx
	sysretq

#===========================================================
# Receive a message on message port RDI
# RSI = Buffer to return message to
# If there is no message on the port block and wait for one
#===========================================================
Receive_Message:
	push %rcx
	call ReceiveMessage
	pop %rcx
	sysretq

#==================================================
# Deallocate the memory at location RDI.
# This will deallocate both user and kernel memory
#==================================================
Dealloc_Mem:
	push %rcx
	call DeallocMem
	pop %rcx
	sysretq

#======================================================
# Send a message to a message port and receive a reply
# RDI = message port
# RSI = message
#======================================================
Send_Receive:
	push %rcx
	call SendReceiveMessage
	pop %rcx
	sysretq

#=========================================================
# Kill the current task, freeing all memory owned by task
# This syscall should never return
#=========================================================
Kill_Task:
	push %rcx
	call KillTask
	pop  %rcx
	sysretq

Halt:
	sti
	hlt
	sysretq

#=================================================================
# Create a new kernel task from the code pointed to by RDI
# Affects RAX, RDI
# Note - we really, really shouldn't have a system call to create
# a new kernel task! On the other hand, it calls code compiled
# into the kernel, so no real harm done.
#=================================================================
NewKerneltask:
	push %rcx
	call NewKernelTask
	int $20
	pop %rcx
	sysretq

#==============================================================
# Allocate some shared memory. RDI = amount to allocate
# Returns in RAX address of allocated memory.
#==============================================================
Alloc_Shared_Mem:
	push %rcx
	call AllocSharedMem
	pop  %rcx
	sysretq

#==============================================================
# Return the value of CR3 in RAX
#==============================================================
GetCR3:
	mov %cr3, %rax
	sysretq

#=================================================================
# Create the low-priority task from the code pointed to by RDI
# Affects RAX, RDI
# Note - we really, really shouldn't have a system call to create
# this task! We only want it to happen once.
#=================================================================
NewLPtask:
	push %rcx
	call NewLowPriTask
	int $20
	pop %rcx
	sysretq
