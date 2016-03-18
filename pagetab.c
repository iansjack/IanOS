#include <kernel.h>
#include <pagetab.h>

extern long nPagesFree;
extern long firstFreePage;
extern unsigned short int *PMap;

extern long kernelPT;
extern long virtualPDP;

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

unsigned int GetPTIndex(l_Address lAddress)
{
	return lAddress >> 12 & 0x1FF;
}

unsigned int GetPDIndex(l_Address lAddress)
{
	return lAddress >> 21 & 0x1FF;
}

unsigned int GetPDPIndex(l_Address lAddress)
{
	return lAddress >> 30 & 0x1FF;
}

unsigned int GetPML4Index(l_Address lAddress)
{
	return lAddress >> 39 & 0x1FF;
}

//=========================================================================
// Return the value of PTE corresponding to address lAddress
//=========================================================================
p_Address checkPTE(l_Address lAddress)
{
	unsigned int ptIndex = GetPTIndex(lAddress);
	unsigned int pdIndex = GetPDIndex(lAddress);
	unsigned int pdpIndex = GetPDPIndex(lAddress);
	unsigned int pml4Index = GetPML4Index(lAddress);
	p_Address pdp, pd, pt;

	struct PML4 *pml4 = (struct PML4 *) (PAGE(currentTask->cr3));

	pdp = PAGE(VIRT(PML4,pml4) ->entries[pml4Index].value);
	if (!pdp)
		return 0;
	pd = PAGE(VIRT(PDP,pdp) ->entries[pdpIndex].value);
	if (!pd)
		return 0;
	pt = PAGE(VIRT(PD,pd) ->entries[pdIndex].value);
	return 0;
	if ((VIRT(PT,pt) ->entries[ptIndex].value))
		return PAGE((VIRT(PT,pt) ->entries[ptIndex].value));
	return 0;
}

void AllocateRange(l_Address lAddress, long size, unsigned short pid)
{
	l_Address startAddress = PAGE(lAddress);
	l_Address endAddress = lAddress + size;

	while (startAddress < endAddress)
	{
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
	unsigned int pdIndex = GetPDIndex(lAddress);
	unsigned int pdpIndex = GetPDPIndex(lAddress);
	unsigned int pml4Index = GetPML4Index(lAddress);
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
	unsigned int pdpIndex = lAddress >> 30 & 0x1FF;
	unsigned int pml4Index = lAddress >> 39 & 0x1FF;
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
	unsigned int pml4Index = GetPML4Index(lAddress);

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
	if (checkPTE(lAddress))
		return(checkPTE(lAddress));
	p_Address pAddress = AllocPage(pid);
	char * l = (char *) pAddress + VAddr;
	int i;
	for (i = 0; i < PageSize; i++)
		*l++ = 0;
	return CreatePTE(pAddress, lAddress, pid, flags);
}

//================================================================
// Create a Page Table Entry in the Page Table pointed to by pml4
//================================================================
p_Address CreatePTEWithPT(struct PML4 *pml4, p_Address pAddress, l_Address lAddress,
		unsigned short pid, short flags)
{
	unsigned int ptIndex = GetPTIndex(lAddress);
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
p_Address CreatePTE(p_Address pAddress, l_Address lAddress, unsigned short pid, short flags)
{
	p_Address retVal = 0;
	struct PML4 *pml4 = (struct PML4 *) (PAGE(currentTask->cr3));
	if (pid)
		flags |= 0x800;						// Use bit 11 to mark user memory
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
	struct PT *pt = GetPT((struct PML4 *) (PAGE(currentTask->cr3)), UserCode,
			currentTask->pid);
	int i = 0;
	while (VIRT(PT, pt) ->entries[i].value)
	{
		ClearBit(VIRT(PT,pt) ->entries[i].value >> 12);
		nPagesFree++;
		VIRT(PT,pt) ->entries[i++].value = 0;
	}

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

		return (mem);
	}
	return (0);
}

//=================================================================
// Allocate a page of memory and map it to the logical address RSI
// Also map the page to the process RDI
// Map it in that process to logical address RDX
//=================================================================
void AllocSharedPage(unsigned short pid, l_Address lAddress1, l_Address lAddress2)
{
	p_Address page = AllocAndCreatePTE(lAddress1, currentTask->pid, 7);
	CreatePTEWithPT((struct PML4 *) (PidToTask(pid)->cr3), page, lAddress2, pid, 7);
}
