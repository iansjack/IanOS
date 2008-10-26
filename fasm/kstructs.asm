	use64
struc Tss
{
.:
	.nexttask	dq 0
	.waiting	db 0	; 0 = free to run, 1 = waiting
	.rax dq 0
	.rbx dq 0
	.rcx dq 0
	.rdx dq 0
	.rbp dq 0
	.rsi dq 0
	.rdi dq 0
	.rsp dq 0
	.r8  dq 0
	.r9  dq 0
	.r10 dq 0
	.r11 dq 0
	.r12 dq 0
	.r13 dq 0
	.r14 dq 0
	.r15 dq 0
	.rflags dq 0
	.ds  dw 0
	.es  dw 0
	.fs  dw 0
	.gs  dw 0
	.ss  dw 0
	.cr3 dq 0
	.firstdata	dq 0
	.firstfreemem	dq 0
	.nextpage	dq 0
	.size = $ - .
}

virtual at r15 + TaskStruct
	TS Tss
end virtual

struc MessagePort
{
.:
	.waitingProc	 dq	 0
	.msgQueue	 dq	 0
	.size = $ - .
}

virtual at r14
	MP MessagePort
end virtual

struc Message
{
.:
	.nextMessage	dq	0
	.byte		db	0
	.quad		dq	0
	.size = $ - .
}

virtual at r13
	Msg Message
end virtual