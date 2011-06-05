#include "memory.h"

void CreatePT164(struct PT *);
void CreatePT264(struct PT *, struct PML4 *);

//================================================================================
// Create a Page Directory with the necessary entries in the first Page Table
// return the Physical Address of this Page Directory in eax
// This only works in 32 bit mode before paging is enabled
//================================================================================
struct PML4 * CreatePageDir()
{
   struct PML4 * pml4 = (struct PML4 *)AllocPage32(-1);
   struct PDP * pdp = (struct PDP *)AllocPage32(-1);
   struct PD * pd = (struct PD *)AllocPage32(-1);
   struct PT * pt1 = (struct PT *)AllocPage32(-1);
   struct PT * pt2 = (struct PT *)AllocPage32(-1);
   pml4->entries[0].Hi = 0;
   pdp->entries[0].Hi = 0;
   pd->entries[0].Hi = 0;
   pd->entries[1].Hi = 0;
   pml4->entries[0].Lo = (long)pdp | P | RW | US;
   pdp->entries[0].Lo = (long)pd | P | RW | US;
   pd->entries[0].Lo = (long)pt1 | P | RW | US;
   pd->entries[1].Lo = (long)pt2 | P | RW | US;
   CreatePT164(pt1);
   CreatePT264(pt2, pml4);
   return pml4;
}

//=====================================================================
// Create the OS Page Table referred to by the above Page Directory
// This does no remapping - each Logical Adress is mapped to the same
// Physical Address. This covers Physical Addresses from 0 to 0x200000.
// EAX = Physical Address of Page Table
//=====================================================================
void CreatePT164(struct PT * pt)
{
   unsigned char *PMap = (unsigned char *) PageMap;
   int count;

   for (count = 0; count < 0x10; count++)
   {
      pt->entries[count].Hi = 0;
      pt->entries[count].Lo = (count << 12) | P;
   }
   for (count = 0x10; count < 0x200; count++)
      if (PMap[count] && PMap[count] != 0xFF)
      {
         pt->entries[count].Hi = 0;
         pt->entries[count].Lo = (count << 12) | P | RW | US; // Are these permissions right for all entries?
      }
}
//==========================================
// Create the Page Table for the Page Tables
// EAX = PA of level 4 table
// EBX = PA of level 1 table 2
//==========================================
void CreatePT264(struct PT * pt, struct PML4 * pml4)
{
   pt->entries[0].Hi = 0;
   pt->entries[0].Lo = (long)pml4 | P | RW | US;
   pt->entries[1].Hi = 0;
   struct PDP * pdp = (struct PDP *)(pml4->entries[0].Lo & 0xFFFFF000);
   pt->entries[1].Lo = (long)pdp | P | RW | US;
   struct PD * pd = (struct PD *)(pdp->entries[0].Lo & 0xFFFFF000);
   pt->entries[2].Hi = 0;
   pt->entries[2].Lo = (long)pd | P | RW | US;
   struct PT * pt1 = (struct PT *)(pd->entries[0].Lo & 0xFFFFF000);
   pt->entries[3].Hi = 0;
   pt->entries[3].Lo = (long)pt1 | P | RW | US;
   pt1 = (struct PT *)(pd->entries[1].Lo & 0xFFFFF000);
   pt->entries[4].Hi = 0;
   pt->entries[4].Lo = (long)pt1 | P | RW | US;
}
