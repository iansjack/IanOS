	.global Disable_Cursor
	.global Enable_Cursor
	.global Position_Cursor

Disable_Cursor:
	movw $0x3D4, %dx
	movb $0xA, %al
	out  %al, %dx
	movw $0x3D5, %dx
	movb $0x20, %al
	out  %al, %dx
	ret

Enable_Cursor:
	movw $0x3D4, %dx
	movb $0xA, %al
	out  %al, %dx
	movw $0x3D5, %dx
	movb $0x0, %al
	out  %al, %dx
	ret

Position_Cursor: # %rdi = row, %rsi = column
	mov  %rdi, %rbx
	imul $80, %rbx
	add  %rsi, %rbx
	movw $0x3D4, %dx
	movb $0x0F, %al
	out  %al, %dx
	movw $0x3D5, %dx
	mov	 %bl, %al
	out  %al, %dx
	movw $0x3D4, %dx
	movb $0x0E, %al
	out  %al, %dx
	movw $0x3D5, %dx
	mov  %bh, %al
	out  %al, %dx
	ret


