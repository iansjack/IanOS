#include "kstructs.h"
#include "memory.h"
#include "pagetab.h"

#define VIRT(type, name) ((struct type *) ((long) name + VAddr))

extern long nPagesFree;
extern unsigned short int *PMap;
long kernelPT;
long virtualPDP;
extern struct Task *currentTask;

void Debug()
{
}

GetPTIndex(long lAddress)
{
	return lAddress >> 12 & 0x1FF;
}

GetPDIndex(long lAddress)
{
	return lAddress >> 21 & 0x1FF;
}

GetPDPIndex(long lAddress)
{
	return lAddress >> 30 & 0x1FF;
}

GetPML4Index(long lAddress)
{
	return lAddress >> 39 & 0x1FF;
}

//=========================================================================
// Return the physical address of the PT corresponding to address lAddress
//=========================================================================
long GetPT(struct PML4 *pml4, long lAddress, unsigned short pid)
{	
	int pdIndex = GetPDIndex(lAddress);
	int pdpIndex = GetPDPIndex(lAddress);
	int pml4Index = GetPML4Index(lAddress);
	
	struct PDP * pdp = (struct PDP *) (VIRT(PML4,pml4)->entries[pml4Index].value & 0xFFFFF000);
	if (!pdp)
	{
		long newpage = (long)AllocPage(pid);
		VIRT(PML4,pml4)->entries[pml4Index].value = newpage | P | RW | US;
		pdp = (struct PDP*)newpage;
	}
	struct PD *pd = (struct PD *) (VIRT(PDP,pdp)->entries[pdpIndex].value & 0xFFFFF000);
	if (!pd)
	{
		long newpage = (long)AllocPage(pid);
		VIRT(PDP,pdp)->entries[pdpIndex].value = newpage | P | RW | US;
		pd = (struct PD *)newpage;
	}
	struct PT * pt = (struct PT *) (VIRT(PD,pd)->entries[pdIndex].value & 0xFFFFF000);
	if (!pt)
	{		
		long newpage = (long)AllocPage(pid);
		VIRT(PD,pd)->entries[pdIndex].value = newpage | P | RW | US;
		pt = (struct PT *)newpage;
	}
	return (long)pt & 0xFFFFF000;
}

//=========================================================================
// Return the physical address of the PD corresponding to address lAddress
//=========================================================================
long GetPD(struct PML4 *pml4, long lAddress, unsigned short pid)
{	
	int pdpIndex = lAddress >> 30 & 0x1FF;
	int pml4Index = lAddress >> 39 & 0x1FF;

	struct PDP * pdp = (struct PDP *) (VIRT(PML4,pml4)->entries[pml4Index].value & 0xFFFFF000);
	if (!pdp)
	{
		long newpage = (long)AllocPage(pid);
		VIRT(PML4,pml4)->entries[pml4Index].value = newpage | P | RW | US;
		pdp = (struct PDP*)newpage;
	}
	struct PD *pd = (struct PD *) (VIRT(PDP,pdp)->entries[pdpIndex].value & 0xFFFFF000);
	if (!pd)
	{
		long newpage = (long)AllocPage(pid);
		VIRT(PDP,pdp)->entries[pdpIndex].value = newpage | P | RW | US;
		pd = (struct PD *)newpage;
	}
	return (long)pd & 0xFFFFF000;
}

//=========================================================================
// Return the physical address of the PDP corresponding to address lAddress
//=========================================================================
long GetPDP(struct PML4 *pml4, long lAddress, unsigned short pid)
{	
	int pml4Index = GetPML4Index(lAddress);

	struct PDP * pdp = (struct PDP *) (VIRT(PML4,pml4)->entries[pml4Index].value & 0xFFFFF000);
	if (!pdp)
	{
		long newpage = (long)AllocPage(pid);
		VIRT(PML4,pml4)->entries[pml4Index].value = newpage | P | RW | US;
		pdp = (struct PDP*)newpage;
	}
	return (long)pdp &0xFFFFF000;
}

//=====================================================
// Create a Page Table for a new process
// Return a pointer to the Page Directory of this table
//=====================================================
void * VCreatePageDir(unsigned short pid, unsigned short parentPid)
{
	// Allocate the base page for the Page Table
	struct PML4 *pml4 = (struct PML4 *) AllocPage(pid);
	
	// A Page for user code memory entries
	VIRT(PD,GetPD(pml4, UserCode, pid))->entries[GetPDIndex(UserCode)].value = (long) AllocPage(pid) | P | RW | US;
	// A Page for user data memory entries
	VIRT(PD,GetPD(pml4, UserData, pid))->entries[GetPDIndex(UserData)].value = (long) AllocPage(pid) | P | RW | US;
	// A Page for user stack memory entries
	VIRT(PD,GetPD(pml4, UserStack, pid))->entries[GetPDIndex(UserStack)].value = (long) AllocPage(pid) | P | RW | US;
	// A Page for kernel memory entries
	VIRT(PD,GetPD(pml4, KernelStack, pid))->entries[GetPDIndex(KernelStack)].value = (long) AllocPage(pid) | P | RW;
	VIRT(PML4,pml4)->entries[GetPML4Index(VAddr)].value = virtualPDP | P | RW;	// Physical to virtual addresses
	struct PD *pd = (struct PD *)GetPD(pml4, 0, pid);
	VIRT(PD,pd)->entries[GetPDIndex(0)].value = kernelPT | P | RW;			// Kernel entries


	if (parentPid == 0) // Just create some default PTEs
		                // We need these two entries so that NewKernelTask can
						// access the data and stack pages of the new process.
	{
		long c;
		struct PT *pt = (struct PT *)GetPT(pml4, UserData, pid);
		VIRT(PT,pt)->entries[GetPTIndex(UserData)].value = AllocAndCreatePTE(TempUserData, pid);
		c = TempUserData;
		asm ("invlpg %0;"
				:
				:"m"(*(char *)TempUserData)
			 	 );

		pt = (struct PT *)GetPT(pml4, UserStack, pid);
		VIRT(PT,pt)->entries[GetPTIndex(UserStack)].value = AllocAndCreatePTE(TempUStack, pid);
		c = TempUStack;
		asm ("invlpg %0;"
				:
				:"m"(*(char *)TempUStack)
			 	 );
	}
	else // Create PTEs and copy pages based on parent PT
	{	
		// Get physical address of current PT
		struct PML4 *current_pml4 = (struct PML4 *)currentTask->cr3;
		
		// Copy memory pages of existing UserCode and UserData
		
		// Process UserCode
		// Get the physical address of the pointer to UserCode in the current table
		struct PT *pt = (struct PT *)GetPT(pml4, UserCode, pid);
		struct PT *currentPT = (struct PT *)GetPT(current_pml4, UserCode, parentPid);
		int i = GetPTIndex(UserCode);
		long c = TempUserCode;
		while (VIRT(PT,currentPT)->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt)->entries[i].value = AllocAndCreatePTE(TempUserCode, pid);
			// Copy the physical memory
			copyMem(((VIRT(PT, currentPT)->entries[i].value) & 0xFFFFF000) + VAddr,
				 	((VIRT(PT, pt)->entries[i].value) & 0xFFFFF000) + VAddr,
				  	PageSize);
			i++;
		}

		// Process UserData
		// Get the physical address of the pointer to UserCode in the current table
		pt = (struct PT *)GetPT(pml4, UserData, pid);
		currentPT = (struct PT *)GetPT(current_pml4, UserData, parentPid);
		i = GetPTIndex(UserData);
		while (VIRT(PT,currentPT)->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt)->entries[i].value = AllocAndCreatePTE(TempUserCode, pid);
			// Copy the physical memory
			copyMem(((VIRT(PT, currentPT)->entries[i].value) & 0xFFFFF000) + VAddr,
				 	((VIRT(PT, pt)->entries[i].value) & 0xFFFFF000) + VAddr,
				  	PageSize);
			i++;
		}

		// Process UserStack
		// Get the physical address of the pointer to UserStack in the current table
		pt = (struct PT *)GetPT(pml4, UserStack, pid);
		currentPT = (struct PT *)GetPT(current_pml4, UserStack, parentPid);
		i = GetPTIndex(UserStack);
		while (VIRT(PT,currentPT)->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt)->entries[i].value = AllocAndCreatePTE(TempUserCode, pid);
			// Copy the physical memory
			copyMem(((VIRT(PT, currentPT)->entries[i].value) & 0xFFFFF000) + VAddr,
				 	((VIRT(PT, pt)->entries[i].value) & 0xFFFFF000) + VAddr,
				  	PageSize);
			i--;
		}

		// Process KernelStack
		// Get the physical address of the pointer to KernelStack in the current table
		pt = (struct PT *)GetPT(pml4, KernelStack, pid);
		currentPT = (struct PT *)GetPT(current_pml4, KernelStack, parentPid);
		i = GetPTIndex(KernelStack);
		while (VIRT(PT,currentPT)->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt)->entries[i].value = AllocAndCreatePTE(TempUserCode, pid);
			// Copy the physical memory
			copyMem(((VIRT(PT, currentPT)->entries[i].value) & 0xFFFFF000) + VAddr,
				 	((VIRT(PT, pt)->entries[i].value) & 0xFFFFF000) + VAddr,
				  	PageSize);
			i--;
		}
	}
	return (void *) pml4;
}

//=====================================================
// Create a Page Table Entry in the current Page Table
// Allocate the physical page
//=====================================================
long AllocAndCreatePTE(long lAddress, unsigned short pid)
{
	void *pAddress = AllocPage(pid);
	return CreatePTE(pAddress, lAddress, pid);
}

//================================================================
// Create a Page Table Entry in the Page Table pointed to by pml4
//================================================================
long CreatePTEWithPT(struct PML4 *pml4, void *pAddress, long lAddress, unsigned short pid)
{	
	int ptIndex = GetPTIndex(lAddress);;
	struct PT *pt = (struct PT *)GetPT(pml4, lAddress, pid);

	// We don't want this function to be interrupted.
	asm ("cli");
	VIRT(PT,pt)->entries[ptIndex].value = ((long) pAddress & 0xFFFFF000) | RW | US | P;
	asm ("sti");

	return ((long) pAddress | 7);
}

//=====================================================
// Create a Page Table Entry in the current Page Table
//=====================================================
long CreatePTE(void *pAddress, long lAddress, unsigned short pid)
{
	long retVal = 0;
	struct PML4 *pml4 = (struct PML4 *)(currentTask->cr3 & 0xFFFFF000);
	retVal = CreatePTEWithPT(pml4, pAddress, lAddress, pid);
	asm ("invlpg %0;"
		:
		:""(lAddress)
		);
	return retVal;
}

//=========================================
// Allocates one page of memory.
// Returns a pointer to the allocated page.
//=========================================
void * AllocPage(unsigned short int PID)
{
	long i = 0;

	while (PMap[i] != 0) i++;
	PMap[i] = PID;
	i = i << 12;
	nPagesFree--;

	return ((void *) i);
}

//=========================================
// Allocates one page of memory.
// Returns a pointer to the allocated page.
//=========================================
void * AllocPageNoClear(unsigned short int PID)
{
	long i = 0;

	while (PMap[i] != 0) i++;
	PMap[i] = PID;
	i = i << 12;
	nPagesFree--;
	
	return ((void *) i);
}
