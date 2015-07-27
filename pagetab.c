#include <kernel.h>
#include <pagetab.h>

#define VIRT(type, name) ((struct type *) ((long) name + VAddr))

extern long nPagesFree;
extern long firstFreePage;
extern unsigned short int *PMap;
long kernelPT;		// Initialized in ptab32.c
long virtualPDP;	// Initialized in ptab32.c
extern struct Task *currentTask;

void Debug()
{
}

void SetBit(int count)
{
	int i = count / 8;
	int j = count % 8;
	PMap[i] |= 1 << j;
}

void ClearBit(int count)
{
	int i = count /8;
	int j = count % 8;
	PMap[i] &= ~(1 << j);
}

int GetBit(int count)
{
	int i = count / 8;
	int j = count % 8;
	if (PMap[i] & (1 << j))
		return 1;
	else
		return 0;
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
struct PT *GetPT(struct PML4 *pml4, long lAddress, unsigned short pid)
{
	int pdIndex = GetPDIndex(lAddress);
	int pdpIndex = GetPDPIndex(lAddress);
	int pml4Index = GetPML4Index(lAddress);
	long pdp, pd, pt;

	pdp = VIRT(PML4,pml4) ->entries[pml4Index].value & 0xFFFFF000;
	if (!pdp)
	{
		long newpage = (long) AllocPage(pid);
		VIRT(PML4,pml4) ->entries[pml4Index].value = newpage | P | RW | US;
		pdp = newpage;
	}
	pd = VIRT(PDP,pdp) ->entries[pdpIndex].value & 0xFFFFF000;
	if (!pd)
	{
		long newpage = (long) AllocPage(pid);
		VIRT(PDP,pdp) ->entries[pdpIndex].value = newpage | P | RW | US;
		pd = newpage;
	}
	pt = VIRT(PD,pd) ->entries[pdIndex].value & 0xFFFFF000;
	if (!pt)
	{
		long newpage = (long) AllocPage(pid);
		VIRT(PD,pd) ->entries[pdIndex].value = newpage | P | RW | US;
		pt = newpage;
	}
	return (struct PT *) (pt & 0xFFFFF000);
}

//=========================================================================
// Return the physical address of the PD corresponding to address lAddress
//=========================================================================
struct PD *GetPD(struct PML4 *pml4, long lAddress, unsigned short pid)
{
	int pdpIndex = lAddress >> 30 & 0x1FF;
	int pml4Index = lAddress >> 39 & 0x1FF;
	long pdp, pd;

	pdp = VIRT(PML4,pml4) ->entries[pml4Index].value & 0xFFFFF000;
	if (!pdp)
	{
		long newpage = (long) AllocPage(pid);
		VIRT(PML4,pml4) ->entries[pml4Index].value = newpage | P | RW | US;
		pdp = newpage;
	}
	pd = VIRT(PDP,pdp) ->entries[pdpIndex].value & 0xFFFFF000;
	if (!pd)
	{
		long newpage = (long) AllocPage(pid);
		VIRT(PDP,pdp) ->entries[pdpIndex].value = newpage | P | RW | US;
		pd = newpage;
	}
	return (struct PD *) (pd & 0xFFFFF000);
}

//=========================================================================
// Return the physical address of the PDP corresponding to address lAddress
//=========================================================================
struct PDP *GetPDP(struct PML4 *pml4, long lAddress, unsigned short pid)
{
	int pml4Index = GetPML4Index(lAddress);

	long pdp = VIRT(PML4,pml4) ->entries[pml4Index].value & 0xFFFFF000;
	if (!pdp)
	{
		long newpage = (long) AllocPage(pid);
		VIRT(PML4,pml4) ->entries[pml4Index].value = newpage | P | RW | US;
		pdp = newpage;
	}
	return (struct PDP *) (pdp & 0xFFFFF000);
}

//=====================================================
// Create a Page Table for a new process
// Return a pointer to the Page Directory of this table
//=====================================================
void * VCreatePageDir(unsigned short pid, unsigned short parentPid)
{
	struct PML4 *pml4;
	struct PD *pd;

	// Allocate the base page for the Page Table
	pml4 = (struct PML4 *) AllocPage(pid);

	VIRT(PML4,pml4) ->entries[GetPML4Index(VAddr)].value = virtualPDP | P
			| RW; // Physical to virtual addresses
	pd = GetPD(pml4, 0, pid);
	VIRT(PD,pd) ->entries[GetPDIndex(0)].value = kernelPT | P | RW; // Kernel entries

	if (parentPid == 0) // Just create some default PTEs
	// We need these two entries so that NewKernelTask can
	// access the data and stack pages of the new process.
	{
		long c;
		struct PT *pt = GetPT(pml4, UserData, pid);
		VIRT(PT,pt) ->entries[GetPTIndex(UserData)].value =
				AllocAndCreatePTE(TempUserData, pid, RW | P);
		c = TempUserData;

		asm ("invlpg %0;"
				:
				:"m"(*(char *)TempUserData)
		);

		pt = GetPT(pml4, KernelStack, pid);
		VIRT(PT,pt) ->entries[GetPTIndex(KernelStack)].value =
				AllocAndCreatePTE(TempUStack, pid, RW | P);

		pt = GetPT(pml4, UserStack, pid);
		VIRT(PT,pt) ->entries[GetPTIndex(UserStack)].value =
				AllocAndCreatePTE(TempUStack, pid, RW | P);
		c = TempUStack;
		asm ("invlpg %0;"
				:
				:"m"(*(char *)TempUStack)
		);
	}
	else // Create PTEs and copy pages based on parent PT
	{
		// Get physical address of current PT
		struct PML4 *current_pml4 = (struct PML4 *) (currentTask->cr3
				& 0xFFFF000);

		// Copy memory pages of existing UserCode and UserData

		// Process UserCode
		// Get the physical address of the pointer to UserCode in the current table
		struct PT *pt = GetPT(pml4, UserCode, pid);
		struct PT *currentPT = GetPT(current_pml4, UserCode, parentPid);
		int i = GetPTIndex(UserCode);
		while (VIRT(PT,currentPT) ->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt) ->entries[i].value = AllocAndCreatePTE(TempUserCode,
					pid, US | RW | P);
			// Copy the physical memory
			memcpy(
					(void *) ((VIRT(PT, pt) ->entries[i].value) & 0xFFFFF000)
							+ VAddr,
					(void *) ((VIRT(PT, currentPT) ->entries[i].value)
							& 0xFFFFF000) + VAddr,
					PageSize);
			i++;
		}

		// Process UserData
		// Get the physical address of the pointer to UserCode in the current table
		pt = GetPT(pml4, UserData, pid);
		currentPT = GetPT(current_pml4, UserData, parentPid);
		i = GetPTIndex(UserData);
		while (VIRT(PT,currentPT) ->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt) ->entries[i].value = AllocAndCreatePTE(TempUserCode,
					pid, US | RW | P);
			// Copy the physical memory
			memcpy(
					(void *) ((VIRT(PT, pt) ->entries[i].value) & 0xFFFFF000)
							+ VAddr,
					(void *) ((VIRT(PT, currentPT) ->entries[i].value)
							& 0xFFFFF000) + VAddr,
					PageSize);
			i++;
		}

		// Process UserStack
		// Get the physical address of the pointer to UserStack in the current table
		pt = GetPT(pml4, UserStack, pid);
		currentPT = GetPT(current_pml4, UserStack, parentPid);
		i = GetPTIndex(UserStack);
		while (VIRT(PT,currentPT) ->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt) ->entries[i].value = AllocAndCreatePTE(TempUserCode,
					pid, US | RW | P);
			// Copy the physical memory
			memcpy(
					(void *) ((VIRT(PT, pt) ->entries[i].value) & 0xFFFFF000)
							+ VAddr,
					(void *) ((VIRT(PT, currentPT) ->entries[i].value)
							& 0xFFFFF000) + VAddr,
					PageSize);
			i--;
		}

		// Process KernelStack
		// Get the physical address of the pointer to KernelStack in the current table
		pt = GetPT(pml4, KernelStack, pid);
		currentPT = GetPT(current_pml4, KernelStack, parentPid);
		i = GetPTIndex(KernelStack);
		while (VIRT(PT,currentPT) ->entries[i].value)
		{
			// Create a page table entry in the new Page Table and also point TempUserCode to it.
			VIRT(PT,pt) ->entries[i].value = AllocAndCreatePTE(TempUserCode,
					pid, US | RW | P);
			// Copy the physical memory
			memcpy(
					(void *) ((VIRT(PT, pt) ->entries[i].value) & 0xFFFFF000)
							+ VAddr,
					(void *) ((VIRT(PT, currentPT) ->entries[i].value)
							& 0xFFFFF000) + VAddr,
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
long AllocAndCreatePTE(long lAddress, unsigned short pid, short flags)
{
	void *pAddress = AllocPage(pid);
	char * l = (char *)pAddress + VAddr;
	int i;
	for (i = 0; i < PageSize; i++)
		*l++ = 0;
	return CreatePTE(pAddress, lAddress, pid, flags);
}

//================================================================
// Create a Page Table Entry in the Page Table pointed to by pml4
//================================================================
long CreatePTEWithPT(struct PML4 *pml4, void *pAddress, long lAddress,
		unsigned short pid, short flags)
{
	int ptIndex = GetPTIndex(lAddress);
	struct PT *pt = GetPT(pml4, lAddress, pid);		// <=== The return value from this looks wrong

	// We don't want this function to be interrupted.
	asm ("pushf");
	asm ("cli");

	VIRT(PT,pt) ->entries[ptIndex].value = ((long) pAddress & 0xFFFFF000)
			| flags;
	asm ("popf");

	return ((long) pAddress | flags);
}

//=====================================================
// Create a Page Table Entry in the current Page Table
//=====================================================
long CreatePTE(void *pAddress, long lAddress, unsigned short pid, short flags)
{
	long retVal = 0;
	struct PML4 *pml4 = (struct PML4 *) (currentTask->cr3 & 0xFFFFF000);
	retVal = CreatePTEWithPT(pml4, pAddress, lAddress, pid, flags);
#ifndef S_SPLINT_S
	asm ("invlpg %0;"
			:
			:""(lAddress)
	);
#endif
	return retVal;
}

void ClearUserMemory(void)
{
	// UserCode
	struct PT *pt = GetPT((struct PML4 *) (currentTask->cr3 & 0xFFFFF000),
			UserCode, currentTask->pid);
	int i = 0;
	while (VIRT(PT, pt) ->entries[i].value)
	{
		ClearBit(VIRT(PT,pt) ->entries[i].value >> 12);
		nPagesFree++;
		VIRT(PT,pt) ->entries[i++].value = 0;
	}

	// User Data
	pt = GetPT((struct PML4 *) (currentTask->cr3 & 0xFFFFF000), UserData,
			currentTask->pid);
	i = 0;
	while (VIRT(PT, pt) ->entries[i].value)
	{
		ClearBit(VIRT(PT,pt) ->entries[i].value >> 12);
		nPagesFree++;
		VIRT(PT,pt) ->entries[i++].value = 0;
	}

	// Let's forget about the stacks
	return;
}

//=========================================
// Allocates one page of memory.
// Returns a pointer to the allocated page.
//=========================================
void * AllocPage(unsigned short int PID)
{
	if (nPagesFree > 100)
	{
		long count = firstFreePage;
		void *mem;

		while (GetBit(count))
			count++;
		SetBit(count);
		mem = (void *) (count << 12);
		nPagesFree--;

		// Zero-fill page
		for (count = 0; count < PageSize; count++)
			((char *)mem + VAddr)[count] = 0;

		return (mem);
	}
	return (0);
}

//=================================================================
// Allocate a page of memory and map it to the logical address RSI
// Also map the page to the process RDI
// Map it in that process to logical address RDX
//=================================================================
void AllocSharedPage(unsigned short pid, long lAddress1, long lAddress2)
{
	long page = AllocAndCreatePTE(lAddress1, currentTask->pid, 7);
	CreatePTEWithPT((struct PML4 *)(PidToTask(pid)->cr3), (void *)page, lAddress2, pid, 7);
}
