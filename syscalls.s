	SLEEPINT = 2

	.include "include/memory.inc"
	.include "include/kstructs.inc"
	.include "macros.s"

	.text

	.global SysCalls
	.global SetCurrentDirectory

CallNo:
	.quad	Sys_Exit
	.quad	Sys_Fork
	.quad	Sys_Read
	.quad	Sys_Write
	.quad	Sys_Open
	.quad	Sys_Close	
	.quad	SysWaitPID
	.quad	Sys_Creat
	.quad	Unimplemented	#Sys_Link
	.quad	Sys_UnLink
	.quad	Sys_Execve
	.quad	Sys_ChDir
	.quad	Sys_Time
	.quad	Unimplemented	#Sys_MkNod
	.quad	Unimplemented	#Sys_ChMod
	.quad	Unimplemented	#Sys_LChOwn
	.quad	Unimplemented	#Unused
	.quad	Sys_Stat
	.quad	Sys_LSeek
	.quad	GetPid
	.quad	Unimplemented	#Sys_Mount
	.quad	Unimplemented	#Sys_Umount
	.quad	Unimplemented	#Sys_SetUID
	.quad	Unimplemented	#Sys_GetUID
	.quad	Unimplemented	#Sys_Stime
	.quad	Unimplemented	#Sys_Ptrace
	.quad	Sys_Alarm
	.quad	Sys_FStat
	.quad	GetTicks
	.quad	Sys_Nanosleep
	.quad	Alloc_Mem
	.quad	Send_Message
	.quad	Receive_Message
	.quad	Dealloc_Mem
	.quad	Send_Receive
	.quad	GetCurrentConsole
	.quad 	Sys_Getcwd
	.quad	Sys_MkDir
	.quad	Alloc_Page
	.quad	Sys_Truncate
	.quad	Sys_QueuePacket
	.quad	Sys_ReceivePacket
	.quad	Sys_GetNetPort
	.quad	Sys_AllocMessagePort
	.quad	Alloc_Shared_Page

SysCalls:
	jmp *(CallNo - 8)(,%r9, 8)

Alloc_Page:
	push %rcx
	mov  currentTask, %r15
	cmp $0, %rdi
	jne alloc
	mov TS.firstfreemem(%r15), %rax
	pop %rcx
	sysretq
alloc:
	mov  TS.pid(%r15), %rsi
	mov	 $7, %rdx
	call AllocAndCreatePTE
	pop %rcx
	sysretq

GetPid:
	mov currentTask, %r15
	mov TS.pid(%r15), %rax
	sysretq

#=========================================================
# Kill the current task, freeing all memory owned by task
# This syscall should never return
#=========================================================
Sys_Exit:
	push %rcx
	call KillTask
	pop  %rcx
	sysretq

#=========================================================
# Creates an exact copy of the current task
# Returns in RAX either the pid of the new file or 0
# when returning to the new file. Note that this function
# returns both in the calling program and the copy.
#=========================================================	
Sys_Fork:
	push %rcx
	call DoFork
	pop %rcx
	sysretq
	
#=========================================================
# Reads from the file whose FCB is in RDI
# RSI = buffer to read into
# RDX = number of bytes to read
# Returns actual number read in RAX
#=========================================================	
Sys_Read:
	push %rcx
	call DoRead
	pop %rcx
	sysretq
	
#=========================================================
# Writes to the file whose FCB is in RDI
# RSI = buffer to write from
# RDX = number of bytes to write
# Returns actual number written in RAX
#=========================================================
Sys_Write:
	push %rcx
	call DoWrite
	pop %rcx
	sysretq
	
#=========================================================
# Opens the file whose name is pointed to by RDI.
# Returns in RAX the File Descriptor
#=========================================================
Sys_Open:
	push %rcx
	call DoOpen
	pop %rcx
	sysretq
	
#=========================================================
# Closes the file whose FD is pointed to by RDI
#=========================================================
Sys_Close:
	push %rcx
	call DoClose
	pop %rcx
	sysretq				

#=========================================================
# Tells this process to wait for the task with pid RDI
# to finish. The task will be sent a message when this
# happens
#=========================================================
SysWaitPID:
	push %rcx
	call Do_Wait
	pop %rcx
	sysretq
	
#=========================================================
# Create a new file whose name is pointed to by RDI
#=========================================================
Sys_Creat:
	push %rcx
	call DoCreate
	pop %rcx
	sysretq

#=========================================================
# Unimplemented
#=========================================================
Unimplemented:	 #Sys_Link:
	sysretq

#=========================================================
# Delete the file whose name is pointed to by RDI
#=========================================================
Sys_UnLink:
	push %rcx
	call DoDelete
	pop %rcx
	sysretq

#=========================================================
# Replace the current task code with that in the file
# pointed to by RAX
#=========================================================
Sys_Execve:
	push %rcx
	call DoExec
	cmp $0,%rax
	jne notLoaded
	mov $(UserStack + PageSize), %rsp
	push $UserCode
notLoaded:
	pop %rcx
	sysretq

#=========================================================
# Changes to the directory whose name is pointed to by RDI
#=========================================================
Sys_ChDir:
	push %rcx
	call DoChDir
	pop %rcx
	sysretq				

#=========================================================
# Returns information about the file whose name is pointed
# to by RDI to the buffer pointed to by RSI
#=========================================================

Sys_Stat:
	push %rcx
	call DoStat
	pop %rcx
	sysretq

#=========================================================
# Returns information about the file whose FCB is in RDI
# to the buffer pointed to by RSI
#=========================================================
Sys_FStat:
	push %rcx
	call DoFStat
	pop %rcx
	sysretq

#=========================================================
# Changes the buffer cursor in the FCB RDI to the offset
# in RSI. RDX determines where the offset is from.
#=========================================================
Sys_LSeek:
	push %rcx
	call DoSeek
	pop %rcx
	sysretq

#=============================================================================
# Return in RAX the number of (10ms) clock ticks since the system was started
#=============================================================================
GetTicks:
	mov Ticks, %rax
	sysretq

#=====================================================
# Suspend the current task for for RDI 10ms intervals
#=====================================================
Sys_Nanosleep:	
	push %rcx
	call GoToSleep
	pop %rcx
	sysretq

#==============================================================
# Allocate some memory from the heap. RDI = amount to allocate
# Returns in RAX address of allocated memory.
#==============================================================
Alloc_Mem:
	push %rcx
	push %r15
	mov  currentTask, %r15
	mov  TS.firstfreemem(%r15), %rsi
	call AllocMem
	pop %r15
	pop  %rcx
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

#=================================================
# Returns the console (0 - 3) of the current task
#=================================================
GetCurrentConsole:
	push %r15
	mov currentTask, %r15
	mov TS.console(%r15), %rax
	pop %r15
	sysretq

#===================================================
# Returns the current directory of the current task
#===================================================
Sys_Getcwd:
	push %rcx
	call DoGetcwd
	pop %rcx
	sysretq

#========================================================
# Creates a directory
# RSI = name
#========================================================
Sys_MkDir:
	push %rcx
	call DoMkDir
	pop %rcx
	sysretq
	
#============================================================
# Returns the system time in the variable pointed to by %rdi
#============================================================
Sys_Time:
	mov unixtime, %rax
#	mov %rax, (%rdi)
	sysretq

#=========================================================
# Truncates file RDI to length RSI
#=========================================================
Sys_Truncate:
	push %rcx
	call DoTruncate
	pop %rcx
	sysretq

Sys_QueuePacket:
	push %rcx
	call queue_packet
	pop %rcx
	sysretq

Sys_ReceivePacket:
	push %rcx
	call receive_packet
	pop %rcx
	sysretq

Sys_GetNetPort:
	mov NetPort, %rax
	sysretq

Sys_AllocMessagePort:
	push %rcx
	call AllocMessagePort
	pop %rcx
	sysretq

Sys_Alarm:
	push %rcx
	call newtimer
	pop %rcx
	sysretq

#=================================================================
# Allocate a page of memory and map it to the logical address RSI
# Also map the page to the process RDI
# Map it in that process to logical address RDX
#=================================================================
Alloc_Shared_Page:
	push %rcx
	call AllocSharedPage
	pop %rcx
	sysretq
