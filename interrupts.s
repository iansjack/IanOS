	.include "macros.s"
	.include "include/memory.inc"
	.include "include/kstructs.inc"

KBDINT 		= 1
SLEEPINT 	= 2
HDINT 		= 3

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
	.global div0
	.global i1
	.global i2
	.global i3
	.global i4
	.global i5
	.global i6
	.global i7
	.global df
	.global i9
	.global ia
	.global ib
	.global ic

#===================
# Keyboard interrupt
#===================
KbInt:
	PUSH_ALL
	in   $0x60, %al		 # MUST read byte from keyboard - else no more ints
	mov  $kbBuffer, %rbx
   	add  kbBufCurrent, %rbx
	mov  %al, (%rbx)
	incl kbBufCurrent
   	incb kbBufCount
   	cmpl $128, kbBufCurrent
	jne  .notoverflow
   	movl $0, kbBufCurrent
.notoverflow:
   	call keyPressed
	mov  $0x20, %al		  # clear int
	out  %al, $0x20
	POP_ALL
	iretq

#================
# Timer interrupt
#================
TimerInt:
	PUSH_ALL
	mov  $0x20, %al
	out  %al, $0x20
	incq Ticks
	# Check for tasks waiting on timer
	mov  blockedTasks, %rbx
	cmp $0, %rbx
	jz   .notimer
.again:
	mov 8(%rbx), %r15
	cmpb $SLEEPINT, TS.waiting(%r15)
	jne  .next
	decq TS.timer(%r15)
	jnz  .next
	mov  %r15, %rdi
	push %rbx
   	call UnBlockTask
   	pop %rbx
.next:
	mov  (%rbx), %rbx
	cmp $0, %rbx
   jne  .again
.notimer:
	decb TimeSliceCount
	jnz  .tdone
	movb $10, TimeSliceCount
	cmp $0, canSwitch
	jnz .tdone
	SWITCH_TASKS
.tdone:
	POP_ALL
//	SWITCH_TASKS
	iretq

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
	PUSH_ALL
	call UnBlockTask
	POP_ALL
	SWITCH_TASKS
	jmp  .done2
.goon:
	mov  TS.nexttask(%r15), %r15
	cmpq $0, %r15
	jne  .again2
.done2:
	pop  %rax
	iretq

#===========================================================================================================
# We need to wrap these two subroutines inside interrupt routines.
# Thus we can call them from, e.g., a SYSCALL. (They expect to be called from within an interrupt routine.)
#===========================================================================================================
SwitchTasks:				# int 20
	cli
	call TaskSwitch
	iretq

SpecificSwitchTasks:		# int 22
	cli
	call SpecificTaskSwitch
	iretq

intr:
	KWRITE_STRING $Unknown_message, $1, $0
intrr:
	KWRITE_STRING $rax, $9, $60
	KWRITE_DOUBLE %rax, $9, $68
	KWRITE_STRING $rbx, $10, $60
	KWRITE_DOUBLE %rbx, $10, $68
	KWRITE_STRING $rcx, $11, $60
	KWRITE_DOUBLE %rcx, $11, $68
	KWRITE_STRING $rdx, $12, $60
	KWRITE_DOUBLE %rdx, $12,$68
	KWRITE_STRING $rip, $0, $60
	KWRITE_DOUBLE 0x20(%rsp), $0, $68
	KWRITE_STRING $cs, $1, $60
	KWRITE_DOUBLE 0x28(%rsp), $1, $68
	KWRITE_STRING $flags, $2, $60
	KWRITE_DOUBLE 0x30(%rsp), $2, $68
	KWRITE_STRING $rsp, $3, $60
	KWRITE_DOUBLE 0x38(%rsp), $3, $68
	KWRITE_STRING $ss, $4, $60
	KWRITE_DOUBLE 0x40(%rsp), $4, $68
	cmpl $UserCode, 0x20(%rsp)
	jge kcode
	KWRITE_STRING $usererror, $14, $58
	call KillTask
kcode:
	cli
	hlt
	iretq

div0:
	KWRITE_STRING $div0message, $0, $0
	jmp intrr

i1:	KWRITE_STRING $debugmessage, $0, $0
	jmp intrr

i2:	KWRITE_STRING $nmimessage, $0, $0
	jmp intrr

i3:	KWRITE_STRING $breakpointmessage, $0, $0
	jmp intrr

i4:	KWRITE_STRING $overflowmessage, $0, $0
	jmp intrr

i5:	KWRITE_STRING $boundmessage, $0, $0
	jmp intrr

i6:	KWRITE_STRING $invalidopcodemessage, $0, $0
	jmp intrr

i7:	KWRITE_STRING $devnotavailmessage, $0, $0
	jmp intrr

df:	KWRITE_STRING $DFmessage, $0, $0
	KWRITE_STRING $error, $6, $60
	KWRITE_DOUBLE 0x20(%rsp), $6, $68
	add $8, %rsp
	jmp intrr

i9:	KWRITE_STRING $coprocmessage, $0, $0
	jmp intrr

ia:	KWRITE_STRING $invalidtssmessage, $0, $0
	KWRITE_STRING $error, $6, $60
	KWRITE_DOUBLE 0x20(%rsp), $6, $68
	add $8, %rsp
	jmp intrr

ib:	KWRITE_STRING $segnotpresentmessage, $0, $0
	KWRITE_STRING $error, $6, $60
	KWRITE_DOUBLE 0x20(%rsp), $6, $68
	add $8, %rsp
	jmp intrr

ic:	KWRITE_STRING $stackfaultmessage, $0, $0
	KWRITE_STRING $error, $6, $60
	KWRITE_DOUBLE 0x20(%rsp), $6, $68
	add $8, %rsp
	jmp intrr

gpf:
	KWRITE_STRING $GPFmessage, $0, $0
	KWRITE_STRING $error, $6, $60
	KWRITE_DOUBLE 0x20(%rsp), $6, $68
	add $8, %rsp
	jmp intrr

pf:
	KWRITE_STRING $PFmessage, $0, $0
	KWRITE_STRING $error, $6, $60
	KWRITE_DOUBLE 0x20(%rsp), $6, $68
	add $8, %rsp
	KWRITE_STRING $address, $7, $60
	KWRITE_DOUBLE %cr2, $7, $68
	jmp intrr

itf:
	movb $'f, 0xB8000
	jmp intr

ig:	movb $'g, 0xB8000
	jmp intr

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
	cmpxchg %rbx, (%rdi)
	jne  .sdone
   	pop  %rbx
   	pop  %rax
	SWITCH_TASKS
	jmp .sagain
.sdone:
	pop  %rbx
	pop  %rax
	ret

#===========================================
# Clear a semaphore whose address is in %rdi
#===========================================
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
