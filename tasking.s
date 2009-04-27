	.include "memory.h"
	.include "kstructs.h"

	.text

	.global TaskSwitch
	.global SpecificTaskSwitch
	.global CreateKernelTask

#===================================================================================================
# Save the current task, restore the next ready one (if there is one!), and initiate a task switch.
# If there isn't another runnable task, and the current task is still runnable, just continue.
# If there's no runnable task at all then halt the processor until the next interrupt.
#===================================================================================================
TaskSwitch:
.again:
	mov  currentTask, %r15
	push %r15
	push %rax
	push %rdx
	call moveTaskToEndOfQueue
	pop  %rdx
	pop  %rax
# Is there another task ready to run?
.nexttask:
	mov  runnableTasksHead, %r15
	cmp  $0, %r15
	jne  TS1
# There's no other runnable task, so pick the low-priority task
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
	push %rcx
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

	.data

	.global currentTask
	.global TSS64

TSS64:	.long 0
	.long tempstack0
	.rept 0x2C
	.long 0
	.endr
