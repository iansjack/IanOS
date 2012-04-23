#include <memory.h>

void CreatePT164(struct PT *);
void CreatePhysicalToVirtual(struct PML4 *, int);

extern long long virtualPDP;
extern long long kernelPT;
extern long nPages;

//================================================================================
// Create a Page Directory with the necessary entries in the first Page Table.
// return the Physical Address of this Page Directory.
// This only works in 32 bit mode before paging is enabled.
//================================================================================
struct PML4 * CreatePageDir()
{
	struct PML4 * pml4 = (struct PML4 *) AllocPage32(2);
	struct PDP * pdp = (struct PDP *) AllocPage32(2);
	struct PD * pd = (struct PD *) AllocPage32(2);
	struct PT * pt1 = (struct PT *) AllocPage32(1);
	struct PT * pt2 = (struct PT *) AllocPage32(2);
	pml4->entries[0].Hi = 0;
	pdp->entries[0].Hi = 0;
	pd->entries[0].Hi = 0;
	pd->entries[1].Hi = 0;
	pml4->entries[0].Lo = (long) pdp | P | RW | US;
	pdp->entries[0].Lo = (long) pd | P | RW | US;
	pd->entries[0].Lo = (long) pt1 | P | RW;
	pd->entries[1].Lo = (long) pt2 | P | RW;
	CreatePT164(pt1);
	CreatePhysicalToVirtual(pml4, nPages);
	kernelPT = (long long) ((long) pt1);
	return pml4;
}

//=====================================================================
// Create the OS Page Table referred to by the above Page Directory
// This does no remapping - each Logical Address is mapped to the same
// Physical Address. This covers Physical Addresses from 0 to 0x200000.
//=====================================================================
void CreatePT164(struct PT * pt)
{
	unsigned short int *PMap = (unsigned short int *) PageMap;
	int count;

	pt->entries[0].Hi = 0;
	pt->entries[0].Lo = P | RW;
	for (count = 1; count < 0x10; count++)
	{
		pt->entries[count].Hi = 0;
		pt->entries[count].Lo = (count << 12) | P | RW | G;
	}
	for (count = 0x10; count < 0x200; count++)
		if (PMap[count] == 1)
		{
			pt->entries[count].Hi = 0;
			pt->entries[count].Lo = (count << 12) | P | RW | G;
		}
}

//==========================================================
// Create Page Table entries mapping all physical addresses
// to PAddr + 0x8000000000
//==========================================================
void CreatePhysicalToVirtual(struct PML4 * pml4, int noOfPages)
{
	int PTsNeeded = (noOfPages / 512);
	int PDsNeeded = (PTsNeeded / 512) + 1;
	int PDPsNeeded = (PDsNeeded / 512) + 1;
	int count1, count2, count3;

	struct PDP * pdp = (struct PDP *) AllocPage32(1);
	struct PD * pd;
	struct PT * pt;

	virtualPDP = (long long) ((long) pdp);
	pml4->entries[1].Lo = (long) pdp | P | RW;

	for (count3 = 0; count3 < PDsNeeded; count3++)
	{
		pd = (struct PD *) AllocPage32(1);
		pdp->entries[count3].Lo = (long) pd | P | RW;

		for (count2 = 0; count2 < PTsNeeded; count2++)
		{
			pt = (struct PT *) AllocPage32(1);
			pd->entries[count2].Lo = (long) pt | P | RW;

			for (count1 = 0; count1 < 0x200; count1++)
				pt->entries[count1].Lo = (count3 << 30) + (count2 << 21)
						+ (count1 << 12) | P | RW | G;
		}
	}
}
