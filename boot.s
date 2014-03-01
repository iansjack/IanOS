#===============================================================================
# This Boot Sector borrows heavily from "Write Your Own 32-bit Operating System
#===============================================================================
	.include "macros.s"
	.include "include/memory.inc"

	.code16
	.org	0
	.global _start

_start:
	jmp Bootup
	nop

Herald:			.ascii "MYOSVER1"
nBytesPerSect:	.word 0x0200
nSectPerClstr:	.byte 0x01
nRsvdSect:		.word 0x0001
nFATS:			.byte 0x02
nRootDirEnts:	.word 0x00E0
nTotalSectors:	.word 0x0B40
bMedia:			.byte 0xF0
nSectPerFAT:	.word 0x0009
nSectPerTrack:	.word 0x0012
nHeads:			.word 0x0002
nHidden: 		.long 0x00000000
nTotalSect32:	.long 0x00000000
bBootDrive:		.byte 0x00
ResvdByte:		.byte 0x00
ExtBootSig:		.byte 0x29
nOSSectors:		.word 0x0100	# Make sure this is enough! 0x 80 sectors takes us to 0x10000 (+0x1000)
ResvsWord:		.word 0x3F51
Volname:		.ascii "RICH       "
FatType:		.ascii "FAT12   "

# Relocate boot sector to 0x90000
Bootup:
	cli
	mov $0x9000, %ax
	mov %ax, %ss
	mov $0x8000, %sp
	mov $0x9000, %ax
	mov %ax, %es
	xor %di, %di
	mov $0x7C0, %ax
	mov %ax, %ds
	xor %si, %si
	mov $512, %cx
	rep movsb

# The next 5 instructions effect a long jump to 0x9000:here
	mov $0x9000, %ax
	push %ax
	mov $here, %ax #6fh
	push %ax
	lret

# The following code calculates the start sector of the first data file and put it in register ax
# This will work out as 0x21, which could have been hard-coded. Doing the calculation makes it
# easier to extend this floppy boot sector to a hard-disk boot sector.
here:
	push %es
	pop %ds
	mov nSectPerTrack, %cx
	xor %ax, %ax
	mov %ax, %ds
	mov $0x0078, %bx
	lds (%bx), %si
	mov %cl, 4(%si)
	movb $0x0f, 9(%si)
	push %es
	pop %ds
	push %ds
	sti
	mov (bBootDrive), %dl
	xor %ax, %ax
	int $0x13
	jc BadBoot
	pop %ds
	mov $MsgLoad, %si
	call PutChars
	xor %ax, %ax
	mov (nFATS), %al
	mulw (nSectPerFAT)
	add (nHidden), %ax
	adc (nHidden+2), %dx
	add (nRsvdSect), %ax
	mov %ax, %cx
	mov $0x20, %ax
	mulw (nRootDirEnts)
	mov (nBytesPerSect), %bx
	div %bx
	add %cx, %ax
	mov (nOSSectors), %cx
	jmp ContinueBoot

BadBoot:
	mov $MsgBadDisk, %si
	call PutChars
	xor %ax, %ax
	int $16
	int $19

PutChars:
	lodsb
	or %al, %al
	jz Done
	mov $0xe, %ah
	mov $0x7, %bx
	int $0x10
	jmp PutChars
Done:
	ret

# Load the 0x80 sectors of OS code to 0x10000
ContinueBoot:
	mov $0x1000, %bx
	mov %bx, %es
NextSector:
	push %ax
	push %cx
	push %dx
	push %es
	xor %bx, %bx
	mov (nSectPerTrack), %si
	div %si
	inc %dl
	mov %dl, (ResvdByte)
	xor %dx, %dx
	divw (nHeads)
	mov (bBootDrive), %dh
	xchg %dl, %dh
	mov %ax, %cx
	xchg %cl, %ch
	shl $6, %cl
	or (ResvdByte), %cl
	mov $1, %al
	mov $2, %ah
	int $0x13
	jc  BadBoot
	mov $MsgDot, %si
	call PutChars		# Print a "." for each sector loaded
	pop %es
	pop %dx
	pop %cx
	pop %ax
	mov %es, %bx
	add $0x20, %bx
	mov %bx, %es
	inc %ax
	loop NextSector
	mov $0x9000, %ax
	mov %ax, %ds

# relocate GDT
	mov $GDT, %ax
	mov %ax, %es
	mov $0, %di
	lea (mygdt), %si
	mov $0x40, %cx
	cld
	rep movsb

#enable_a20:
	xor %cx, %cx
IBEmm0:
	cli
	in  $0x64, %al
	test $2, %al
	jnz IBEmm0
	mov $0xD1, %al
	out %al, $0x64
	xor %cx, %cx

IBEmm1:
	cli
	in  $0x64, %al
	test $2, %al
	jnz IBEmm1
	mov $0xDF, %al
	out %al, $0x60
	xor %cx, %cx

# load register gdt and enable Protected Mode
	lgdt (gdt_48)
	mov %cr0, %eax
	bts $0x0, %eax
	mov %eax, %cr0

# jump to the start of the OS code (startup.s)
	jmpl $0x8, $0x10000

#===================================================
# Global Descriptor Table - will be relocated to 0x0
#===================================================
mygdt:	.quad	0x0	       					# null descriptor - first entry in GDT must be null
	CODE_SEG_DESCR OsCodeSeg,0x0,0xfffff,0	# 32-bit descriptors
	DATA_SEG_DESCR OsDataSeg,0x0,0xfffff
	CODE64_SEG_DESCR code64, 0				# 64-bit descriptors
	DATA64_SEG_DESCR data64, 0
	DATA64_SEG_DESCR udata64, 3
	CODE64_SEG_DESCR user64, 3

GDTlength = . - mygdt

gdt_48: 	.word	0x800	  # allow up to 512 entries in GDT
		.double	0x00000000

MsgBadDisk:	.byte 0x0D,0x0A
		.ascii "Bad Boot Disk!"
		.byte 0x00
MsgLoad: 	.byte 0x0D,0x0A
		.ascii "Loading IanOS"
		.byte 0x00
MsgDot:		.byte '.',0

#====================================================================================
# Ensure that the bootsector is 512 bytes and add a boot signature at the end of it
#====================================================================================

		.org  510
BootSig:	.word 0xAA55
