#include <kernel.h>
#include <pagetab.h>

// #define DEBUG

extern long nPagesFree;
extern long firstFreePage;
extern unsigned short int *PMap;

extern long kernelPT;
extern long virtualPDP;

extern struct Task *currentTask;

#ifdef DEBUG
long allocations[32];
long currAlloc;
#endif

void Debug()
{
}

static inline InvalidatePage(l_Address address)
{
	asm volatile ( "invlpg (%0)" : : "b"(address) : "memory" );
}

void SetBit(int count)
{
	int i = count / 8;
	int j = count % 8;
	PMap[i] |= 1 << j;
}

void ClearBit(int count)
{
	long mem = count << 12;
	int n;

#ifdef DEBUG
	for (n = 0; n < 32; n++)
	if (allocations[n] == mem)
	allocations[n] = 0;
#endif

	int i = count / 8;
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

unsigned long GetPTIndex(l_Address lAddress)
{
	return lAddress >> 12 & 0x1FF;
}

unsigned long GetPDIndex(l_Address lAddress)
{
	return lAddress >> 21 & 0x1FF;
}

unsigned long GetPDPIndex(l_Address lAddress)
{
	return lAddress >> 30 & 0x1FF;
}

unsigned long GetPML4Index(l_Address lAddress)
{
	return lAddress >> 39 & 0x1FF;
}

//=========================================================================
// Return the value of PTE corresponding to address lAddress
//=========================================================================
p_Address checkPTE(l_Address lAddress)
{
	struct PML4 *pml4 = (struct PML4 *) currentTask->cr3;
	return checkPTEWithPT(pml4, lAddress);
}

p_Address checkPTEWithPT(struct PML4 *pml4, l_Address lAddress)
{
	unsigned long ptIndex = GetPTIndex(lAddress);
	unsigned long pdIndex = GetPDIndex(lAddress);
	unsigned long pdpIndex = GetPDPIndex(lAddress);
	unsigned long pml4Index = GetPML4Index(lAddress);
	p_Address pdp, pd, pt;

	pdp = PAGE(VIRT(PML4,pml4) ->entries[pml4Index].value);
	if (!pdp)
		return 0;
	pd = PAGE(VIRT(PDP,pdp) ->entries[pdpIndex].value);
	if (!pd)
		return 0;
	pt = PAGE(VIRT(PD,pd) ->entries[pdIndex].value);
	if (!pt)
		return 0;
	if ((VIRT(PT,pt) ->entries[ptIndex].value))
		return PAGE((VIRT(PT,pt) ->entries[ptIndex].value));
	return 0;
}

void AllocateRange(l_Address lAddress, long size, unsigned short pid,
		unsigned char user)
{
	l_Address startAddress = PAGE(lAddress);
	l_Address endAddress = lAddress + size;

	while (startAddress < endAddress)
	{
		int flags = RW | US | P;
		if (user)
			flags += 0x800;
		if (!checkPTE(startAddress))
			AllocAndCreatePTE(startAddress, pid, RW | US | P);
		startAddress += PageSize;
	}
}

//=========================================================================
// Return the physical address of the PT corresponding to address lAddress
//=========================================================================
struct PT *GetPT(struct PML4 *pml4, l_Address lAddress, unsigned short pid)
{
	unsigned long pdIndex = GetPDIndex(lAddress);
	unsigned long pdpIndex = GetPDPIndex(lAddress);
	unsigned long pml4Index = GetPML4Index(lAddress);
	p_Address pdp, pd, pt;
	unsigned int flags = P + RW + US;
	if (pid)
		flags |= 0x800;

	pdp = PAGE(VIRT(PML4,pml4) ->entries[pml4Index].value);
	if (!pdp)
	{
		p_Address newpage = AllocPage(pid);
		VIRT(PML4,pml4) ->entries[pml4Index].value = newpage | flags; //P | RW | US;
		pdp = newpage;
	}
	pd = PAGE(VIRT(PDP,pdp) ->entries[pdpIndex].value);
	if (!pd)
	{
		p_Address newpage = AllocPage(pid);
		VIRT(PDP,pdp) ->entries[pdpIndex].value = newpage | flags; // P | RW | US;
		pd = newpage;
	}
	pt = PAGE(VIRT(PD,pd) ->entries[pdIndex].value);
	if (!pt)
	{
		p_Address newpage = AllocPage(pid);
		VIRT(PD,pd) ->entries[pdIndex].value = newpage | flags; // P | RW | US;
		pt = newpage;
	}
	return (struct PT *) (PAGE(pt));
}

//=========================================================================
// Return the physical address of the PD corresponding to address lAddress
//=========================================================================
struct PD *GetPD(struct PML4 *pml4, l_Address lAddress, unsigned short pid)
{
	unsigned long pdpIndex = lAddress >> 30 & 0x1FF;
	unsigned long pml4Index = lAddress >> 39 & 0x1FF;
	p_Address pdp, pd;
	unsigned int flags = P | RW | US;
	if (pid)
		flags |= 0x800;

	pdp = PAGE(VIRT(PML4,pml4) ->entries[pml4Index].value);
	if (!pdp)
	{
		p_Address newpage = AllocPage(pid);
		VIRT(PML4,pml4) ->entries[pml4Index].value = newpage | flags;
		pdp = newpage;
	}
	pd = PAGE(VIRT(PDP,pdp) ->entries[pdpIndex].value);
	if (!pd)
	{
		p_Address newpage = AllocPage(pid);
		VIRT(PDP,pdp) ->entries[pdpIndex].value = newpage | flags;
		pd = newpage;
	}
	return (struct PD *) (PAGE(pd));
}

//=========================================================================
// Return the physical address of the PDP corresponding to address lAddress
//=========================================================================
struct PDP *GetPDP(struct PML4 *pml4, l_Address lAddress, unsigned short pid)
{
	unsigned long pml4Index = GetPML4Index(lAddress);

	p_Address pdp = PAGE(VIRT(PML4,pml4) ->entries[pml4Index].value);
	if (!pdp)
	{
		p_Address newpage = AllocPage(pid);
		VIRT(PML4,pml4) ->entries[pml4Index].value = newpage | P | RW | US;
		pdp = newpage;
	}
	return (struct PDP *) (PAGE(pdp));
}

//=====================================================
// Create a Page Table for a new process
// Return a pointer to the Page Directory of this table
//=====================================================
p_Address VCreatePageDir(unsigned short pid, unsigned short parentPid)
{
	struct PML4 *pml4;
	struct PD *pd;
	int extra = 0;
	if (pid)
		extra = 0x800;

	// Allocate the base page for the Page Table
	pml4 = (struct PML4 *) AllocPage(pid);

	// Physical to virtual addresses
	VIRT(PML4,pml4) ->entries[GetPML4Index(VAddr)].value = virtualPDP | P
			| RW | US;

	pd = GetPD(pml4, 0, pid);
	VIRT(PD,pd) ->entries[GetPDIndex(0)].value = kernelPT | P | RW; // Kernel entries

	return (p_Address) pml4;
}

//=====================================================
// Create a Page Table Entry in the current Page Table
// Allocate the physical page
//=====================================================
p_Address AllocAndCreatePTE(l_Address lAddress, unsigned short pid, short flags)
{
	p_Address ret;

	// If the page is already mapped, just clear the page and return the physical address
	if (checkPTE(lAddress))
	{
		ret = checkPTE(lAddress);
		// Clear the page
		char * l = (char *)lAddress; //(char *) ret + VAddr;
		int i;
		for (i = 0; i < PageSize; i++)
			*l++ = 0;
		return ret;
	}

	// Else, allocate a new page and map it
	ret = AllocPage(pid);
	CreatePTE(ret, lAddress, pid, flags);
	return ret;
}

//================================================================
// Create a Page Table Entry in the Page Table pointed to by pml4
//================================================================
p_Address CreatePTEWithPT(struct PML4 *pml4, p_Address pAddress,
		l_Address lAddress, unsigned short pid, short flags)
{
	unsigned long ptIndex = GetPTIndex(lAddress);
	struct PT *pt = GetPT(pml4, lAddress, pid);	// <=== The return value from this looks wrong

	// We don't want this function to be interrupted.
	asm ("pushf");
	asm ("cli");

	VIRT(PT,pt) ->entries[ptIndex].value = PAGE(pAddress) | flags;
	asm ("popf");

	return (pAddress | flags);
}

//=====================================================
// Create a Page Table Entry in the current Page Table
//=====================================================
p_Address CreatePTE(p_Address pAddress, l_Address lAddress, unsigned short pid,
		short flags)
{
	p_Address retVal = 0;
	struct PML4 *pml4 = (struct PML4 *) (PAGE(currentTask->cr3));
	if (pid)
		flags |= 0x800;						// Use bit 11 to mark user memory
	retVal = CreatePTEWithPT(pml4, pAddress, lAddress, pid, flags);
#ifndef S_SPLINT_S
	InvalidatePage(lAddress);
#endif
	return retVal;
}

void ClearUserMemory(void)
{
	// UserCode - No!!! Don't clear User Code
	struct PT *pt = GetPT((struct PML4 *) (PAGE(currentTask->cr3)), UserCode,
			currentTask->pid);
	int i = 0;
	//while (VIRT(PT, pt) ->entries[i].value)
	//{
	//	ClearBit(VIRT(PT,pt) ->entries[i].value >> 12);
	//	nPagesFree++;
	//	VIRT(PT,pt) ->entries[i++].value = 0;
	//}

	// User Data
	pt = GetPT((struct PML4 *) (PAGE(currentTask->cr3)), UserData,
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

//=====================================================
// Allocates one page of memory.
// Returns the physical address of the allocated page.
//=====================================================
p_Address AllocPage(unsigned short int PID)
{
	if (nPagesFree > 100)
	{
		long count = firstFreePage;
		p_Address mem;

		while (GetBit(count))
			count++;
		SetBit(count);
		mem = count << 12;
		nPagesFree--;

		// Zero-fill page
		for (count = 0; count < PageSize; count++)
			((char *) mem + VAddr)[count] = 0;

#ifdef DEBUG
		if (currAlloc < 32) allocations[currAlloc++] = mem;
		if (mem == 0x8d6000)
		{
			asm("cli");
			asm("hlt");
		}
#endif

		return (mem);
	}
	return (0);
}

//=================================================================
// Allocate a page of memory and map it to the logical address RSI
// Also map the page to the process RDI
// Map it in that process to logical address RDX
//=================================================================
void AllocSharedPage(unsigned short pid, l_Address lAddress1,
		l_Address lAddress2)
{
	p_Address page = AllocAndCreatePTE(lAddress1, currentTask->pid, 7);
	CreatePTEWithPT((struct PML4 *) (PidToTask(pid)->cr3), page, lAddress2, pid,
			7);
}

//================================================================
// Copy the given page from the current page table to a new one
// Return the logical address of the page, or zero if the
// page doesn't exist
//================================================================
l_Address CopyPage(l_Address address, struct PML4 *pml4, unsigned short pid)
{
	// Page align the address
	address = PAGE(address);

	// Get physical address of page tables
	struct PML4 *current_pml4 = (struct PML4 *) (currentTask->cr3);
	struct PT *pt = GetPT(pml4, address, pid);
	struct PT *currentPT = GetPT(current_pml4, address, currentTask->pid);
	unsigned long i = GetPTIndex(address);
	if (!(VIRT(PT,currentPT) ->entries[i].value))
		return 0;

	// Create and map the new page and copy the physical memory
	if (!checkPTEWithPT(pml4, address))
	{
		p_Address data = AllocPage(pid);
		CreatePTEWithPT(pml4, data, address, pid, US | RW | P | 0x800);
	}
	memcpy((void *) PAGE(((VIRT(PT, pt)) ->entries[i].value)) + VAddr,
			(void *) PAGE(((VIRT(PT, currentPT)) ->entries[i].value))
					+ VAddr, PageSize);
	return address;
}

//===================================================================
// Makes an entry in the page table pointing to the same page as the
// entry in the current page table. Mark it as a non-user page
// Return the logical address of the page, or zero if the
// page doesn't exist
//===================================================================
l_Address DuplicatePage(l_Address address, struct PML4 *pml4,
		unsigned short pid)
{
	// Page align the address
	address = PAGE(address);

	// Get physical address of page tables
	struct PML4 *current_pml4 = (struct PML4 *) (currentTask->cr3);
	struct PT *pt = GetPT(pml4, address, pid);
	struct PT *currentPT = GetPT(current_pml4, address, currentTask->pid);
	unsigned long i = GetPTIndex(address);
	if (!(VIRT(PT,currentPT) ->entries[i].value))
		return 0;

	// Create and map the new page and copy the physical memory
	if (!checkPTEWithPT(pml4, address))
		VIRT(PT, pt) ->entries[i].value =
				VIRT(PT, currentPT) ->entries[i].value & 0xFFFFFFFFFFFFF8FF;
	return address;
}

void ClearDirtyBit(l_Address address)
{
	address = PAGE(address);

	// Get physical address of page tables
	struct PML4 *current_pml4 = (struct PML4 *) (currentTask->cr3);
	struct PT *pt = GetPT(current_pml4, address, currentTask->pid);
	struct PT *currentPT = GetPT(current_pml4, address, currentTask->pid);
	unsigned long i = GetPTIndex(address);
	(VIRT(PT,currentPT) ->entries[i].value) &= 0xFFFFFFFFFFFFFFBF;
	InvalidatePage(address);
}

FlushDisk()
{
	struct PML4 *currentPT = (struct PML4 *) (currentTask->cr3);
	int i;
	for (i = 256; i < 512; i++)
	{
		struct PDP *pdp = (struct PDP *)(PAGE(((VIRT(PDP, currentPT))->entries[i].value)));
		if (pdp)
		{
			int j;
			for (j = 0; j < 512; j++)
			{
				struct PD *pd = (struct PD *)(PAGE(((VIRT(PD, pdp))->entries[j].value)));
				if (pd)
				{
					int k;
					for (k = 0; k < 512; k++)
					{
						struct PT *pt = (struct PT *)(PAGE(((VIRT(PT, pd))->entries[k].value)));
						if (pt)
						{
							int l;
							for (l = 0; l < 512; l++)
							{
								long entry = ((VIRT(PT, pt))->entries[l].value);
								if (entry & 0x40)	// "Dirty" bit is set
								{
									long address = i << 9;
									address = (address + j) << 9;
									address = (address + k) << 9;
									address = (address + l) << 12;
									address = address | 0xFFFF000000000000;
									WritePageToDisk(address);
									InvalidatePage(address);
								}
							}
						}
					}
				}
			}
		}
	}
}
