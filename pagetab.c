#include "memory.h"
#include "pagetab.h"

extern long nPagesFree;
extern unsigned char *PMap;

/* Oops! I think we need to do this before we start paging.
// Create the Page Tables entries mapping Physical memory to Physical + 0x1000000000
long CreatePhysicalMapping(void)
{
   struct PML4e * base;
   base = AllocPage64();
   base->P = 0;
   base->RW = 0;
   base->US = 0;
   base->PWT = 0;
   base->PCD = 0;
   base->A = 0;
   base->ZEROS = 0;
   base->Base = AllocPage64();
   base->AVL = 0;
   base->NX = 0;
   return base;
}
*/

//=====================================================
// Create a Page Table for a new process
// Return a pointer to the Page Directory of this table
//=====================================================
void *
VCreatePageDir(void)
{
   long *TPTL4 = (long *) TempPTL4;
   long *TPTL3 = (long *) TempPTL3;
   long *TPTL2 = (long *) TempPTL2;
   long *TPTL12 = (long *) TempPTL12;
   long *PTL12 = (long *) PageTableL12;

   void *PD = AllocPage();

   CreatePTE(PD, (long) TempPTL4);
   TPTL4[0] = CreatePTE(AllocPage(), TempPTL3);
   TPTL3[0] = CreatePTE(AllocPage(), TempPTL2);
   TPTL2[0] = PTL12[3];
   TPTL2[1] = CreatePTE(AllocPage(), TempPTL12);
   TPTL12[0] = (long) PD + 7;
   TPTL12[1] = TPTL4[0];
   TPTL12[2] = TPTL3[0];
   TPTL12[3] = TPTL2[0];
   TPTL12[4] = TPTL2[1];
   // 1 Page for User Code
   TPTL12[0x100] = CreatePTE(AllocPage(), TempUserCode);
   // 1 Page for User Data
   TPTL12[0x110] = CreatePTE(AllocPage(), TempUserData);
   // 1 Page for kernel stack
   TPTL12[0x1FC] = CreatePTE(AllocPage(), TempKStack);
   // 1 Page for user stack
   TPTL12[0x1FE] = CreatePTE(AllocPage(), TempUStack);
   return (PD);
}

//====================================================
// Create a Page Table Entry in the current Page Table
//====================================================
long
CreatePTE(void *pAddress, long lAddress)
{
   long *PTableL11 = (long *) PageTableL11;

   // We don't want this function to be interrupted.
   asm ("cli");
   PTableL11[lAddress >> 12] = (long) pAddress | 7;
   char *c = (char *) lAddress;
   // These lines update the page table cache.
   // Without them results will be unpredictable.
   asm ("push %rbx");
   asm ("mov %cr3, %rbx");
   asm ("mov %rbx, %cr3");
   asm ("pop %rbx");
   int count;
   for (count = 0; count < PageSize; count++)
      c[count] = 0;
   asm ("sti");

   return ((long) pAddress | 7);
}

//=========================================
// Allocates one page of memory.
// Returns a pointer to the allocated page.
//=========================================
void *
AllocPage()
{
   long i = 0;

   while (PMap[i] != 0)
   {
      i++;
   }
   PMap[i] = 1;
   i = i << 12;
   nPagesFree--;
   return ((void *) i);
}

