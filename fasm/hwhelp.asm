	format ELF

	section '.text'

InBuffEmpty:
	push eax
	push ecx
	mov ecx,2FFFFh
ibe:	in al,64h
	test al,2
	loopnz ibe
	pop ecx
	pop eax
	ret

OutBuffFull:
	push eax
	push ecx
	mov ecx,2FFFFh
obf:	in al,64h
	test al,1
	loopz	obf
	pop ecx
	pop eax
	ret
