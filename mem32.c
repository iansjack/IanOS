#include "memory.h"
//#define PageMap         0x000000000007F000L

extern long nPagesFree;

/*
 #==================================
 # Find top of memory
 #==================================
 InitMemManagement:
 movw $0, (memorySemaphore)
 movl $0, NoOfAllocations
 movl $256, nPagesFree
 movl $0, nPagesFree + 4;
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
 */

extern long memorySemaphore;
extern long NoOfAllocations;
extern long nPagesFree;
extern unsigned char * oMemMax;
long nPages;

//========================================
// Find out how much memory is present.
// Initialize some variables.
// Create PageMap.
//========================================
void
InitMemManagement()
{
   unsigned short int *PMap = (unsigned short int *) PageMap;

   memorySemaphore = 0;
   *(&memorySemaphore + 1) = 0;
   NoOfAllocations = 0;
   *(&NoOfAllocations + 1) = 0;
   nPagesFree = 256;
   *(&nPagesFree + 1) = 0;

   // Find number of free pages by writing a pattern to memory and seeing if it reads back OK
   // We start at 1Mb and go up in 1Mb increments. Each Mb is 256 pages.
   long * mempos = (long *) 0x100000;
   long testpattern = 0x6d72646c;
   while (1)
   {
      *mempos = testpattern;
      if (*mempos != testpattern)
         break;
      //      oMemMax = (unsigned char *)(mempos + 0x60003);
      //      *(&oMemMax + 1) = 0;
      mempos += 0x100000 / (sizeof * mempos);
      nPagesFree += 256;
   }
   nPages = nPagesFree;

   int count;
   for (count = 0; count++; count < 0x4000)
      PMap[count] = 0;

   // GDT and IDT
   PMap[0] = 1;
   nPagesFree--;

   // Kernel Memory
   for (count = 1; count < 18; count++)
   {
      PMap[count] = 1;
      nPagesFree--;
   }

   // Static Message Ports
   PMap[0x6F] = 1;
   nPagesFree--;

   // DiskBuffer
   PMap[0x70] = 1;
   nPagesFree--;

   // PageMap
   PMap[0x7F] = 1;
   nPagesFree--;

   // TaskStructs
   PMap[0x80] = 1;
   nPagesFree--;

   // Ports, ROM, VideoMem, etc.
   for (count = 0xA0; count < 0x100; count++)
   {
      PMap[count] = 1;
      nPagesFree--;
   }

   // Shared Memory
   for (count = 0x1F0; count < 0x200; count++)
   {
      PMap[count] = 1;
      nPagesFree--;
   }

}

//============================================
// Allocate a page of memory and zero fill it
//============================================
void *
AllocPage32(unsigned short int PID)
{
   unsigned short int *PMap = (unsigned short int *) PageMap;
   int count = 0;

   while (PMap[++count])
      ;
   PMap[count] = PID;
   nPagesFree--;
   unsigned char * mem = (unsigned char *)(count << 12);
   for (count = 0; count < PageSize; count++)
      mem[count] = 0;
   return (void *) mem;
}
