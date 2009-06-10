	.include "memory.inc"

	.global InitMemManagement
	.global AllocPage

	.text

	.code32

#==================================
# Find top of memory
#==================================
InitMemManagement:
	movw $0, (memorySemaphore)
	movl $0, NoOfAllocations
	movl $256, nPagesFree
	mov $0x0019FFFC, %eax
	mov $0, %ebx
	mov $0x6d72646c, %ecx
MemLoop:
	mov %ecx, (%eax)
	mov (%eax), %ebx
	cmp %ebx, %ecx
	jne MemLoopEnd
	add $0x00060003, %eax
	mov %eax, oMemMax
	sub $0x00060003, %eax
	add $0x00100000, %eax
	addl $256, nPagesFree
	mov $0, %ebx
	jmp MemLoop
MemLoopEnd:
	mov $0x00004000, %ecx
.a:	movb $0, PageMap(%ecx)
	loop .a
	# Mark used memory in MemoryMap
	# all this is defined as belonging to Task 1

	# GDT and IDT
	movb $1, PageMap
	decl nPagesFree

	# OS Memory
	# mov ecx, DataSegLen
	mov $0x1000, %ecx
	shr $0xB, %ecx
	add $2, %ecx		  # # of pages in OS data segment
	add $0xF, %ecx		  # # of pages in OS code segment
	mov $PageMap, %ebx
.again: 
	movb $1, (%ebx)
	inc %ebx
	decl nPagesFree
	loop .again

	# 0x0006F000 - 0x0006FFFF Static Message Ports
	movb $1, PageMap + 0x6F

	# 0x00070000 - 0x00071000 Disk Buffer
	movb $1, PageMap + 0x70

	# 0x0007F000 - 0x0007FFFF Page Map
	movb $1, PageMap + 0x7F

	# 0x00080000 - 0x00081000, TaskStructs
	movb $1, PageMap + 0x80
	mov  $0x80000, %ebx
	mov  $0x1000, %ecx
.again3:
	movb $0, (%ebx, %ecx)
	loop .again3

	# 0x000A0000 - 0x00100000, Ports, ROM, VideoMem, etc.
	mov $(PageMap + 0xA0), %ebx
	mov $0x60, %ecx
.ag2:	movb $1, (%ebx)
	inc %ebx
	decl nPagesFree
	loop .ag2

	# 0x001F0000 - 0x00200000, Shared Memory
	mov $(PageMap + 0x1F0), %ebx
	mov $0x10, %ecx
.ag3:	movb $1, (%ebx)
	inc %ebx
	decl nPagesFree
	loop .ag3

	ret

#===========================================
# Allocate a page of memory
# AL = task to allocate memory to
# if EAX = 0xFF then memory is for PT
# 0 fill the page
# return in EAX the Physical Memory address
#===========================================
AllocPage:
	push %ebx
	push %ecx
	# find first free page of Physical Memory
	mov $0, %ebx
.again2: cmpb $0, PageMap(%ebx)
	je .found
	inc %ebx
	jmp .again2
	# mark page as in use
.found: mov %al, PageMap(%ebx)
	# compute Physical Address
	xchg %eax, %ebx
	shl $12, %eax
	decl nPagesFree
	#mov  %eax, %ebx
	mov  $0x1000, %ecx
.again4:
	movb $0, (%eax, %ecx)
	loop .again4
	pop %ecx
	pop %ebx
	ret
