#include "memory.h"

extern long long memorySemaphore;
extern long long NoOfAllocations;
extern long long nPagesFree;
extern long long nPages;
extern unsigned char * oMemMax;

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
   nPagesFree = 256;

   // Find number of free pages by writing a pattern to memory and seeing if it reads back OK
   // We start at 1Mb and go up in 1Mb increments. Each Mb is 256 pages.
   long * mempos = (long *) 0x100000;
   long testpattern = 0x6d72646c;
   while (1)
   {
      *mempos = testpattern;
      if (*mempos != testpattern)
         break;
      mempos += 0x100000 / (sizeof *mempos);
      nPagesFree += 256;
   }
   nPages = nPagesFree;

   int count;
   for (count = 0; count++; count < nPages)
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

   // TaskStructs
   PMap[0x80] = 1;
   nPagesFree--;

   // Ports, ROM, VideoMem, etc.
   for (count = 0xA0; count < 0x100; count++)
   {
      PMap[count] = 1;
      nPagesFree--;
   }

   // PageMap
   for (count = 0x100; count < 0x100 + (2 * (long)nPages / (long)PageSize) ; count++)
   {
      PMap[count] = 1;
      nPagesFree--;
   }
}

//============================================
// Allocate a page of memory and zero fill it
//============================================
void *AllocPage32(unsigned short int PID)
{
   unsigned short int *PMap = (unsigned short int *) PageMap;
   int count = 0;

   while (PMap[++count])
      ;
   PMap[count] = PID;
   nPagesFree--;
   unsigned char * mem = (unsigned char *) (count << 12);
   for (count = 0; count < PageSize; count++)
      mem[count] = 0;
   return (void *) mem;
}
