include 'macros.asm'
include 'memory.h'

	use16
org	0

start:
	jmp short Bootup
	nop

Herald		db 'MYOSVER1'
nBytesPerSect	dw 0200h
nSectPerClstr	db 01h
nRsvdSect	dw 0001h
nFATS		db 02h
nRootDirEnts	dw 00E0h
nTotalSectors	dw 0B40h
bMedia		db 0F0h
nSectPerFAT	dw 0009h
nSectPerTrack	dw 0012h
nHeads		dw 0002h
nHidden 	dd 00000000h
nTotalSect32	dd 00000000h
bBootDrive	db 00h
ResvdByte	db 00h
ExtBootSig	db 29h
nOSSectors	dw 80h
ResvsWord	dw 3F51h
Volname 	db 'RICH       '
FatType 	db 'FAT12   '

Bootup:
	cli
	mov ax,9000h
	mov ss,ax
	mov sp,8000h
	mov ax,9000h
	mov es,ax
	xor di,di
	mov ax,7C0h
	mov ds,ax
	xor si,si
	mov cx,512
	rep movsb
	mov ax,9000h
	push ax
	mov ax,here ;6fh
	push ax
	retf
here:
	push es
	pop ds
	mov cx,[nSectPerTrack]
	xor ax,ax
	mov ds,ax
	mov bx,0078h
	lds si,[bx]
	mov byte [si+4],cl
	mov byte [si+9],0fh
	push es
	pop ds
	push ds
	sti
	mov dl,[bBootDrive]
	xor ax,ax
	int 0x13
	jc short BadBoot
	pop ds
	mov si,MsgLoad
	call PutChars
	xor ax,ax
	mov al,[nFATS]
	mul word [nSectPerFAT]
	add ax,word [nHidden]
	adc dx,word [nHidden+2]
	add ax,[nRsvdSect]
	mov cx,ax
	mov ax,0x20
	mul word [nRootDirEnts]
	mov bx,word [nBytesPerSect]
	div bx
	add ax,cx
	mov cx,[nOSSectors]
	jmp short ContinueBoot

BadBoot:
	mov si,MsgBadDisk
	call PutChars
	xor ax,ax
	int 16h
	int 19h

PutChars:
	lodsb
	or al,al
	jz short Done
	mov ah,0xE
	mov bx,0x7
	int 0x10
	jmp short PutChars
Done:
	retn

ContinueBoot:
	mov bx,0x100
	mov es,bx
NextSector:
	push ax
	push cx
	push dx
	push es
	xor bx,bx
	mov si,[nSectPerTrack]
	div si
	inc dl
	mov [ResvdByte],dl
	xor dx,dx
	div word [nHeads]
	mov dh,[bBootDrive]
	xchg dl,dh
	mov cx,ax
	xchg cl,ch
	shl cl,6
	or cl,byte [ResvdByte]
	mov al,1
	mov ah,2
	int 13h
	jc short BadBoot
	mov si,MsgDot
	call PutChars
	pop es
	pop dx
	pop cx
	pop ax
	mov bx,es
	add bx,0x20
	mov es,bx
	inc ax
	loop NextSector
	mov ax,0x9000
	mov ds,ax
; relocate GDT
	mov ax,GDT
	mov es,ax
	mov di,0
	lea si, [mygdt]
	;mov cx, word [gdt_48]
	mov cx, 0x40
	cld
	rep movsb
enable_a20:
	cli
	in  al,0x64
	test al,2
	jnz enable_a20
	mov al,0xBF
	out 0x64,al

	lgdt fword [gdt_48]
	mov eax,cr0
	bts eax, 0
	mov cr0,eax
;	jmp OsCodeSeg:.here
;.here:
	jmp 0x8:0x1000

;===================================================
; Global Descriptor Table - will be relocated to 0x0
;===================================================
mygdt	dq	0h	       ; null descriptor - first entry in GDT must be null
	CODE_SEG_DESCR OsCodeSeg,0x0,0xfffff,0
	DATA_SEG_DESCR OsDataSeg,0x0,0xfffff
	CODE64_SEG_DESCR code64, 0
	DATA64_SEG_DESCR data64, 0
	DATA64_SEG_DESCR udata64, 3
	CODE64_SEG_DESCR user64, 3
	;TSS_SEG_DESCR64 tssd64, TSS64

GDTlength = $ - mygdt

gdt_48: dw	0x800	  ; allow up to 512 entries in GDT
	dd	GDT

MsgBadDisk	db 0xD,0xA,'Bad Boot Disk!',0
MsgLoad 	db 0xD,0xA,'Loading OS',0
MsgDot		db '.',0
