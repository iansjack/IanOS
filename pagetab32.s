PageMap      = 0x000000000007F000

	.global CreatePageDir

	.text

	.code32

#================================================================================
# Create a Page Directory with the necessary entries in the first Page Table
# return the Physical Address of this Page Directory in eax
# This only works in 32 bit mode before paging is enabled
#================================================================================
CreatePageDir:
	mov $0xFF, %eax
	call AllocPage	# for level 4 table
	push %eax
	mov  %eax, %ebx
	mov $0xFF, %eax
	call AllocPage	# for level 3 table
	mov  %eax, (%ebx)
	movl $0, 4(%ebx)
	addl $0x7, (%ebx)
	mov  %eax, %ebx
	mov $0xFF, %eax
	call AllocPage	# for level 2 table
	mov  %eax, (%ebx)
	movl $0, 4(%ebx)
	addl $0x7, (%ebx)
	mov  %eax, %ebx
	mov $0xFF, %eax
	call AllocPage	# for level 1 table 1     # 0x000000 - 0x1FFFFF (OS & stacks)
	mov  %eax, (%ebx)
	movl  $0, 4(%ebx)
	addl  $0x7, (%ebx)
	push %ebx
	call CreatePT164
	pop %ebx
	mov $0xFF, %eax
	call AllocPage	# for level 1 table 2     # 0x200000 - 0x3FFFFF (Page Tables)
	mov %eax, 8(%ebx)
	movl $0, 12(%ebx)
	addl $0x7, 8(%ebx)
	mov %eax, %ebx
	pop %eax
	push %eax
	call CreatePT264
	pop %eax
	ret

#=====================================================================
# Create the OS Page Table referred to by the above Page Directory
# This does no remapping - each Logical Adress is mapped to the same
# Physical Address. This covers Physical Addresses from 0 to 0x200000.
# EAX = Physical Address of Page Table
#=====================================================================
CreatePT164:
	mov %eax, %esi
	mov $0x200, %ecx
	mov $0, %ebx
	mov $1, %eax
	#mov $7, %eax
	mov $PageMap, %edx
again:	movl $0, (%ebx, %esi)
	movl $0, 4(%ebx, %esi)
	cmpb $0, (%edx)
	je  .notused
	cmpb $0xFF, (%edx)
	je  .notused			# well - used by task 1, but we don't want to map it
	mov %eax, (%ebx, %esi)
.notused:
	inc %edx
	add $8, %ebx
	add $0x1000, %eax
.notdata:
	loop again
	ret

#==========================================
# Create the Page Table for the Page Tables
# EAX = PA of level 4 table
# EBX = PA of level 1 table 2
#==========================================
CreatePT264:
	mov %eax, (%ebx)	 # level 4
	addl $7, (%ebx)
	movl $0, 4(%ebx)
	mov (%eax), %eax
	and $0xFFFFF000, %eax
	mov %eax, 8(%ebx) 	# level 3
	addl $7, 8(%ebx)
	movl $0, 12(%ebx)
	mov (%eax), %eax
	and $0xFFFFF000, %eax
	mov %eax, 16(%ebx)	# level 2
	addl $7, 16(%ebx)
	movl $0, 20(%ebx)
	push %eax
	mov (%eax), %eax
	and $0xFFFFF000, %eax
	mov %eax, 24(%ebx)	# level 1 table 1
	addl $7, 24(%ebx)
	movl $0, 28(%ebx)
	pop %eax
	mov 8(%eax), %eax
	and $0xFFFFF000, %eax
	mov %eax, 32(%ebx)	# level 1 table 2
	addl $7, 32(%ebx)
	movl $0, 36(%ebx)
	ret

