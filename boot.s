.include "macros.s"

# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard. We put this into a special
# section so we can force the header to be in the start of the final program.
# You don't need to understand all these details as it is just magic values that
# is documented in the multiboot standard. The bootloader will search for this
# magic sequence and recognize us as a multiboot kernel.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by creating a symbol at the bottom of it,
# then allocating 16384 bytes for it, and finally creating a symbol at the top.
.section .bootstrap_stack
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# The linker script specifies _start as the entry point to the kernel and the
# bootloader will jump to this position once the kernel has been loaded. It
# doesn't make sense to return from this function as the bootloader is gone.
.section .text
.global _start
.type _start, @function
_start:
# relocate GDT
	mov $0, %di
	lea (mygdt), %si
	mov $0x40, %cx
	cld
	rep movsb

# load register gdt
	lgdt (gdt_48)

# Save memory size
	mov 8(%ebx), %ecx
	mov $0x900, %eax
	mov %ecx, (%eax)

    mov 0x18(%ebx), %eax	# address of module

# relocate OS
	mov $0x14000, %ecx
    mov (%eax), %esi
    mov $0x10000, %edi
#    sub %ebx, %ecx
    cld
    rep movsb

    mov $0x10000, %eax
    jmp *%eax

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
