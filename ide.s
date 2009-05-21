HDINT 		= 3
	.global ReadSector
	.global WriteSector

#===========================================================
# Load one sector from IDE HD to memory
# %rdi = address to load to
# %esi = sector number
#===========================================================
HD_PORT=0x1F0

ReadSector:
	mov  $HD_PORT+7, %dx
.again2:
	in  %dx, %al
	test $0x80, %al
	jnz .again2
	mov $HD_PORT+2, %dx		# 0x1F2 - sector count
	mov $1, %al
	out %al, %dx
	inc %dx		      		# 0x1F3
	mov %esi, %eax
	and $0xFF, %al
	out %al, %dx	      		# lba lo
	inc %dx		      		# 0x1F4
	mov %esi, %eax
	and $0xFF, %ah
	mov %ah, %al
	out %al, %dx	      		# lba mid
	inc %dx		      		# 0x1F5
	mov %esi, %eax
	shr $16, %eax
	and $0xFF, %al
	out %al, %dx	      		# lba hi
	inc %dx		      		# 0x1F6
	and $0xF, %ah
	mov %ah, %al
	add $0x40, %al	      		# lba mode/drive /lba top
	out %al, %dx
	inc %dx		      		# 0x1F7
	mov $0x20, %ax			# HDC_READ
	out %al, %dx
	cli
	push %rdi
	push %rdx
	mov $HDINT, %rdi
	call WaitForInt
	pop %rdx
	pop %rdi
.again3:
	in  %dx, %al
	test $0x80, %al
	jnz .again3
	mov $HD_PORT, %dx
	mov $0, %eax
	mov $256, %rcx
	cld
	rep
	insw
	ret

WriteSector:
	mov  $HD_PORT+7, %dx
.again4:
	in  %dx, %al
	test $0x80, %al
	jnz .again4
	mov $HD_PORT+2, %dx		# 0x1F2 - sector count
	mov $1, %al
	out %al, %dx
	inc %dx		      		# 0x1F3
	mov %esi, %eax
	and $0xFF, %al
	out %al, %dx	      		# lba lo
	inc %dx		      		# 0x1F4
	mov %esi, %eax
	and $0xFF, %ah
	mov %ah, %al
	out %al, %dx	      		# lba mid
	inc %dx		      		# 0x1F5
	mov %esi, %eax
	shr $16, %eax
	and $0xFF, %al
	out %al, %dx	      		# lba hi
	inc %dx		      		# 0x1F6
	and $0xF, %ah
	mov %ah, %al
	add $0x40, %al	      		# lba mode/drive /lba top
	out %al, %dx
	inc %dx		      		# 0x1F7
	mov $0x30, %ax			# HDC_WRITE
	out %al, %dx
.again5:
	in  %dx, %al
	test $0x80, %al
	jnz .again5
	mov $HD_PORT, %dx
	mov $0, %eax
	mov $256, %rcx
	mov %rdi, %rsi
	cld
	rep
	outsw
#	cli
#	push %rdi
#	push %rdx
#	mov $HDINT, %rdi
#	call WaitForInt
#	pop %rdx
#	pop %rdi
	ret
