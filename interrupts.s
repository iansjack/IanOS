	.include "macros.s"
	.include "memory.inc"
	.include "kstructs.inc"

KBDINT 	= 1
SLEEPINT = 2
HDINT 	= 3

	.text

	.global WaitForInt
	.global Ps
	.global intr
	.global gpf
	.global pf
	.global SwitchTasks
	.global SpecificSwitchTasks
	.global TimerInt
	.global KbInt
	.global HdInt
	.global SetSem
	.global ClearSem

#===================
# Keyboard interrupt
#===================
KbInt:	push %rax
	push %rbx
	in   $0x60, %al		 # MUST read byte from keyboard - else no more ints
	mov  $kbBuffer, %ebx
	addl kbBufCurrent, %ebx
	mov  %al, (%ebx)
	incl kbBufCurrent
	incb kbBufCount
	cmpl $128, kbBufCurrent
	jne  .kbistaskwaiting
	movl $0, kbBufCurrent

	# is any task waiting for keyboard input? If so re-enable it

.kbistaskwaiting:
	mov  blockedTasks, %r15
.kbagain:
	cmpb $KBDINT, TS.waiting(%r15)
	jne  .kbgoon
	movb $0, TS.waiting(%r15)
	mov  %r15, %rdi
	call UnBlockTask
	SWITCH_TASKS_R15
	jmp  .kbdone
.kbgoon:
	mov  TS.nexttask(%r15), %r15
	cmpq $0, %r15
	jne  .kbagain
.kbdone:
	pop  %rbx
	mov  $0x20, %al		  # clear int
	out  %al, $0x20
	pop  %rax
	iretq

#================
# Timer interrupt
#================
TimerInt:
	push %rax
	mov  $0x20, %al
	out  %al, $0x20
	incq Ticks
	# Check for tasks waiting on timer
	mov  blockedTasks, %r15
	cmp $0, %r15
	jz   .notimer
.again:	cmpb $SLEEPINT, TS.waiting(%r15)
	jne  .next
	decq TS.timer(%r15)
	jnz  .next
	mov  %r15, %rdi
        call UnBlockTask
.next:	mov  TS.nexttask(%r15), %r15
	cmp $0, %r15
        jne  .again
.notimer:
	pop  %rax
	decb TimeSliceCount
	jnz  .tdone
	movb $5, TimeSliceCount
	SWITCH_TASKS
.tdone:	iretq

#=====================
# Hard Disk interrupt
#=====================
HdInt:	push %rax
.istaskwaiting:
	mov  $0x20, %al
	out  %al, $0x20
	out  %al, $0xA0
	mov  blockedTasks, %r15
.again2: 
	cmpb $HDINT, TS.waiting(%r15)
	jne  .goon
	movb $0, TS.waiting(%r15)
	mov  %r15, %rdi
	call UnBlockTask
	SWITCH_TASKS_R15
	jmp  .done2
.goon:	mov  TS.nexttask(%r15), %r15
	cmpq $0, %r15
	jne  .again2
.done2:	pop  %rax
	iretq

#===========================================================================================================
# We need to wrap these two subroutines inside interrupt routines.
# Thus we can call them from, e.g., a SYSCALL. (They expect to be called from within an interrupt routine.)
#===========================================================================================================
SwitchTasks:			# int 20
	cli
	call TaskSwitch
	iretq

SpecificSwitchTasks:		# int 22
	cli
	call SpecificTaskSwitch
	iretq

intr:	KWRITE_STRING $Unknown_message, $0, $0
	KWRITE_DOUBLE 0x20(%esp), $0, $60
	KWRITE_DOUBLE 0x28(%esp), $1, $60
	KWRITE_DOUBLE 0x30(%esp), $2, $60
	KWRITE_DOUBLE 0x38(%esp), $3, $60
	KWRITE_DOUBLE 0x40(%esp), $4, $60
	KWRITE_DOUBLE 0x48(%esp), $5, $60
	pop %rax
	hlt
	iretq

div0:	KWRITE_STRING $div0message, $0, $0
	KWRITE_DOUBLE (%esp), $0, $60
	KWRITE_DOUBLE 4(%esp), $1, $60
	iretq
i1:	movb $'1, 0xB8000
	iretq
i2:	movb $'2, 0xB8000
	iretq
i3:	movb $'3, 0xB8000
	iretq
i4:	movb $'4, 0xB8000
	iretq
i5:	movb $'5, 0xB8000
	iretq
i6:	movb $'6, 0xB8000
	iretq
i7:	movb $'7, 0xB8000
	iretq
i8:	movb $'8, 0xB8000
	iretq
i9:	movb $'9, 0xB8000
	iretq
ia:	movb $'a, 0xB8000
	iretq
ib:	movb $'b, 0xB8000
	iretq
ic:	movb $'c, 0xB8000
	iretq
gpf:
	KWRITE_STRING $GPFmessage, $0, $0
	KWRITE_DOUBLE 0x20(%esp), $0, $60
	KWRITE_DOUBLE 0x28(%esp), $1, $60
	KWRITE_DOUBLE 0x30(%esp), $2, $60
	KWRITE_DOUBLE 0x38(%esp), $3, $60
	KWRITE_DOUBLE 0x40(%esp), $4, $60
	KWRITE_DOUBLE 0x48(%esp), $5, $60
	pop %rax
	hlt
	iretq
pf:	
	KWRITE_STRING $PFmessage, $0, $0
	KWRITE_DOUBLE 0x20(%esp), $0, $60
	KWRITE_DOUBLE 0x28(%esp), $1, $60
	KWRITE_DOUBLE 0x30(%esp), $2, $60
	KWRITE_DOUBLE 0x38(%esp), $3, $60
	KWRITE_DOUBLE 0x40(%esp), $4, $60
	KWRITE_DOUBLE 0x48(%esp), $5, $60
	pop %rax
	hlt
	iretq
itf:	movb $'f, 0xB8000
	iretq
ig:	movb $'g, 0xB8000
	iretq

Pd:	mov $160, %ax
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
	mov %al, 0xB8000(%ebx)
	add $2, %bx
	loop .stillCounting
	ret

Ps:	mov $160, %ax
	mul %bh
	mov $0, %bh
	shl $1, %bx
	add %ax, %bx
.isItStringEnd:
	mov (%edx), %ah
	cmp $0, %ah
	je .done3
	mov %ah, 0xB8000(%ebx)
	add $2, %bx
	inc %edx
	jmp .isItStringEnd
.done3:	ret

#===============================================================================
# Stop the current task and make it wait for the interrupt number passed in RDI
#===============================================================================
WaitForInt:
	mov  currentTask, %r15
	mov  %rdi, %rax
	mov  %al, TS.waiting(%r15)
	mov  %r15, %rdi
	call BlockTask
	sti
	SWITCH_TASKS		       # The current task is no longer runnable
	ret

#==========================================
# Set a semaphore whose address is in %rdi
#==========================================
SetSem:
.sagain:
	push %rax
	push %rbx
	mov  $1, %rbx
	cmpxchg %bx, (%rdi)
	je  .sdone
	SWITCH_TASKS
	jmp .sagain
.sdone:
	pop  %rbx
	pop  %rax
	ret

#==========================================
# Set a semaphore whose address is in %rdi
#==========================================
ClearSem:
	movw $0, (%rdi)
	ret

	.data

	.global Ticks
	.global Timer.active
	.global Timer.interval
	.global Timer.task

Ticks:				.quad 0
TimeSliceCount: 	.byte 5
Timer.active: 		.byte 0
Timer.interval:	.quad 0
Timer.task:			.quad 0
