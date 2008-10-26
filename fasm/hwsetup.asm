	format ELF
	public hwsetup

	section '.text' executable

include 'hwhelp.asm'

hwsetup:
; set up interrupt controller
	mov al,00010001b
	out 20h,al
	out 0a0h,al
	mov al,20h
	out 21h,al
	mov al,28h
	out 0a1h,al
	mov al,00000100b
	out 21h,al
	mov al,00000010b
	out 0a1h,al
	mov al,00000001h
	out 21h,al
	out 0a1h,al
	mov al,11111011b	; disable all interrupts
	out 21h,al
	mov al,11111111b
	out 0a1h,al

; set up keyboard controller
	call InBuffEmpty
	mov al,0FAh
	out 060h,al
	call OutBuffFull
	in al,060h
	call InBuffEmpty
	mov al,0F0h
	out 060h,al
	call OutBuffFull
	in al,060h
	call InBuffEmpty
	mov al,02h
	out 060h,al
	call OutBuffFull
	in al,060h
	call InBuffEmpty
	mov al,060h
	out 064h,al
	call InBuffEmpty
	mov al,45h
	out 060h,al
; enable interrupts
	mov al,11111000b	; enable keyboard + timer interrupt
	out 21h,al

; enable hd interrupt
	mov al,10111111b
	out 0xa1, al

; set up timer hardware for interrupts every 10ms
	mov al, 0x9C
	out 0x40, al
	mov al, 0x2e
	out 0x40, al

HD_PORT=0x1F0
; set up ide controller
.again1:
	in al, dx
	test al, 0x80
	jnz .again1
	mov dx, HD_PORT+6
	mov al, 0x0
	out dx, al
	mov dx, HD_PORT+7
	mov al, 0x10
	out dx, al

	ret
