#include "kstructs.h"
#include "memory.h"
#include "pagetab.h"

#define VIRT(type, name) ((struct type *) ((long) name + 0x8000000000))

extern long nPagesFree;
extern unsigned short int *PMap;
long kernelPT;
long virtualPDP;
extern struct Task *currentTask;

//=====================================================
// Create a Page Table for a new process
// Return a pointer to the Page Directory of this table
//=====================================================
void *
VCreatePageDir(unsigned short pid)
{
   struct PML4 * pml4 = (struct PML4 *) AllocPage(pid);
   struct PDP * pdp = (struct PDP *) AllocPage(pid);
   struct PD * pd = (struct PD *) AllocPage(pid);
   struct PT * pt = (struct PT *) AllocPage(pid);

   VIRT(PML4,pml4)->entries[0].value = (long) pdp | P | RW | US;
   VIRT(PML4,pml4)->entries[1].value = virtualPDP | P | RW | US;
   VIRT(PDP,pdp)->entries[0].value = (long) pd | P | RW | US;
   VIRT(PD,pd)->entries[0].value = kernelPT | P | RW | US;
   VIRT(PD,pd)->entries[1].value = (long) pt | P | RW | US;

   // Entries for the page table itself. We should be able to eliminate these!
   VIRT(PT,pt)->entries[0].value = (long) pml4 + 7;
   VIRT(PT,pt)->entries[1].value = VIRT(PML4,pml4)->entries[0].value;
   VIRT(PT,pt)->entries[2].value = VIRT(PDP,pdp)->entries[0].value;
   VIRT(PT,pt)->entries[3].value = VIRT(PD,pd)->entries[0].value;
   VIRT(PT,pt)->entries[4].value = VIRT(PD,pd)->entries[1].value;

   // 1 Page for User Code
   VIRT(PT,pt)->entries[0x100].value = CreatePTE(AllocPage(pid), TempUserCode);
   // 1 Page for User Data
   VIRT(PT,pt)->entries[0x110].value = CreatePTE(AllocPage(pid), TempUserData);
   // 1 Page for kernel stack
   VIRT(PT,pt)->entries[0x1FC].value = CreatePTE(AllocPage(pid), TempKStack);
   // 1 Page for user stack
   VIRT(PT,pt)->entries[0x1FE].value = CreatePTE(AllocPage(pid), TempUStack);
   return (void *) pml4;
}

//====================================================
// Create a Page Table Entry in the current Page Table
//====================================================
long
CreatePTE(void *pAddress, long lAddress)
{
   int ptIndex = lAddress >> 12 & 0x1FF;
   int pdIndex = lAddress >> 21 & 0x1FF;
   int pdpIndex = lAddress >> 30 & 0x1FF;
   int pml4Index = lAddress >> 39 & 0x1FF;

   // Get the logical address of appropriate PageTable
   struct PML4 * pml4 = (struct PML4 *) (currentTask->cr3 & 0xFFFFF000);
   struct PDP * pdp = (struct PDP *) (VIRT(PML4,pml4)->entries[pml4Index].value
         & 0xFFFFF000);
   struct PD * pd = (struct PD *) (VIRT(PDP,pdp)->entries[pdpIndex].value
         & 0xFFFFF000);
   struct PT * pt = (struct PT *) (VIRT(PD,pd)->entries[pdIndex].value
         & 0xFFFFF000);

   // We don't want this function to be interrupted.
   //asm ("cli");

   VIRT(PT,pt)->entries[ptIndex].value = ((long) pAddress & 0xFFFFF000) | RW
         | US | P;

   // These lines update the page table cache.
   // Without them results may be unpredictable.
   asm ("push %rbx");
   asm ("mov %cr3, %rbx");
   asm ("mov %rbx, %cr3");
   asm ("pop %rbx");

   char *c = (char *) lAddress;
   int count;
   for (count = 0; count < PageSize; count++)
      c[count] = 0;

   //asm ("sti");

   return ((long) pAddress | 7);
}

//=========================================
// Allocates one page of memory.
// Returns a pointer to the allocated page.
//=========================================
void *
AllocPage(unsigned short int PID)
{
   long i = 0;

   while (PMap[i] != 0)
   {
      i++;
   }
   PMap[i] = PID;
   i = i << 12;
   nPagesFree--;
   return ((void *) i);
}
