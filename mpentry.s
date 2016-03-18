.code16
.globl mpentry_start
.global mpentry_end

mpentry_start:
	cli

	xorw    %ax, %ax
	movw    %ax, %ds
	movw    %ax, %es
	movw    %ax, %ss
	movw	$0xB800, %ax
	movw	%ax, %ds
	movw	$0x18, %bx
	movb	$0, %cl
	movw	$0, %dx
here:
	movw	$1024, %ax
again:
	dec		%dx
	cmp		$0, %dx
	jne		again
	dec		%ax
	cmp 	$0, %ax
	jne		again
	movb	%cl, (%bx)
	inc 	%cl
	cmp 	$0, %cl
	jne		here
	hlt
	jmp		here
mpentry_end:
