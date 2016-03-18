	.include "include/memory.inc"
	.include "include/kstructs.inc"

	.text

	.global TaskSwitch
	.global SpecificTaskSwitch
	.global SaveRegisters
	.global ZeroPage

#===================================================================================================
# Save the current task, restore the next ready one (if there is one!), and initiate a task switch.
# If there isn't another runnable task, and the current task is still runnable, just continue.
# If there's no runnable task at all then switch to the low-priority task.
#===================================================================================================
TaskSwitch:
.again:
	mov  currentTask, %r15
	push %r15
	push %rax
	push %rdx
	push %rdi
	mov runnableTasks, %rdi
	call MoveTaskToEndOfList
	mov %rax, runnableTasks
	pop %rdi
	pop  %rdx
	pop  %rax
# Is there another task ready to run?
	mov  runnableTasks, %r15
	cmp  $0, %r15
	je	.loPri
	mov	8(%r15), %r15
	jmp  TS1
# There's no other runnable task, so pick the low-priority task
.loPri:
	mov  lowPriTask, %r15
	jmp  TS1

#==============================================================
# Save the current task and switch to the one specified in R15
#==============================================================
SpecificTaskSwitch:
# Save task state
	push %r15
TS1:	xchg currentTask, %r15
	mov  %rax, TS.rax(%r15)
	mov  %rbx, TS.rbx(%r15)
	mov  %rcx, TS.rcx(%r15)
	mov  %rdx, TS.rdx(%r15)
	mov  %rbp, TS.rbp(%r15)
	mov  %rsi, TS.rsi(%r15)
	mov  %rdi, TS.rdi(%r15)
	mov  %r8,  TS.r8(%r15)
	mov  %r9,  TS.r9(%r15)
	mov  %r10, TS.r10(%r15)
	mov  %r11, TS.r11(%r15)
	mov  %r12, TS.r12(%r15)
	mov  %r13, TS.r13(%r15)
	mov  %r14, TS.r14(%r15)
	pop  %rax
	mov  %rax, TS.r15(%r15)
	mov  %ds,  TS.ds(%r15)
	mov  %cr3, %rax
	mov  %rax, TS.cr3(%r15)
	pop  %rcx
	mov  %rsp, TS.rsp(%r15)
# Restore task state
	mov  currentTask, %r15 	# Actually the task to switch to (which we stored in currentTask)
	mov  TS.cr3(%r15), %rax
	mov  %rax, %cr3
	mov  TS.rsp(%r15), %rsp
	cmpb $1,TS.forking(%r15)
	je   forking			# Forking is a special case
	push %rcx
forking:
	movb $0, TS.forking(%r15) # Don't want to process forking more than once!
	mov  TS.rcx(%r15), %rcx
	mov  TS.rdx(%r15), %rdx
	mov  TS.rbp(%r15), %rbp
	mov  TS.rsi(%r15), %rsi
	mov  TS.rdi(%r15), %rdi
	mov  TS.r8(%r15), %r8
	mov  TS.r9(%r15), %r9
	mov  TS.r10(%r15), %r10
	mov  TS.r11(%r15), %r11
	mov  TS.r12(%r15), %r12
	mov  TS.r13(%r15), %r13
	mov  TS.r14(%r15), %r14
	mov  TS.ds(%r15), %ds
	mov  TS.rax(%r15), %rax
	mov  TS.rbx(%r15), %rbx
	mov  TS.r15(%r15), %r15
	ret
	
SaveRegisters:
	pop  %rsi
	push %rsi
	push %r15	
	mov	 %rdi, %r15
	mov  %rax, TS.rax(%r15)
	mov  %rbx, TS.rbx(%r15)
	mov  %rcx, TS.rcx(%r15)
	mov  %rdx, TS.rdx(%r15)
	mov  %rbp, TS.rbp(%r15)
	mov  %rsi, TS.rsi(%r15)
	mov  %rdi, TS.rdi(%r15)
	mov  %r8,  TS.r8(%r15)
	mov  %r9,  TS.r9(%r15)
	mov  %r10, TS.r10(%r15)
	mov  %r11, TS.r11(%r15)
	mov  %r12, TS.r12(%r15)
	mov  %r13, TS.r13(%r15)
	mov  %r14, TS.r14(%r15)
	mov  %r15, TS.r15(%r15)
	movb  $1, TS.forking(%r15)
	mov  %rsp, %rdi
	add  $0x8, %rdi
	mov  %rdi, TS.rsp(%r15)
	mov  %cr3,%rdi
	mov  TS.cr3(%r15),%rax
	mov  %rax,%cr3
	add  $0x10,%rsp
	push %rsi
	mov  %rdi,%cr3
	sub  $0x8,%rsp
	pop  %r15
	ret

#===========================================================================
# Zero the contents of memory page %rdi
#===========================================================================
ZeroPage:
	push %rcx
	push %rbx
	mov $PageSize / 8, %rcx
	rol $12, %rdi
	bts $Vaddr, %rdi
	cld
	mov $0, %rax
	rep stosq
	pop %rbx
	pop %rcx
	ret
	
#	.data
#
#	.global currentTask
#	.global TSS64
#
#TSS64:
#	.long 0
#	.long tempstack
#	.rept 0x2C
#	.long 0
#	.endr
