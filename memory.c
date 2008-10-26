#include "cmemory.h"
#include "ckstructs.h"

unsigned char oMemMax;
long nPagesFree;
struct MemStruct * firstFreeKMem = (struct MemStruct *)0x11000;	//#dq FFKM
static long nextKPage = 0x12;									//#dq (FFKM shr 12) + 1

unsigned char * PMap = (unsigned char *) PageMap;

/*
=====================================================
Create a Page Table for a new process
Return a pointer to the Page Directory of this table
=====================================================
*/
void * VCreatePageDir()
{
	long * TPTL4 = (long *)TempPTL4;
	long * TPTL3 = (long *)TempPTL3;
	long * TPTL2 = (long *)TempPTL2;
	long * TPTL12 = (long *)TempPTL12;
	long * PTL12 = (long *)PageTableL12; 
	
	void * PD = AllocPage64();

	CreatePTE(PD, (long)TempPTL4);
	TPTL4[0] = CreatePTE(AllocPage64(), TempPTL3);
	TPTL3[0] = CreatePTE(AllocPage64(), TempPTL2);
	TPTL2[0] = PTL12[3];
	TPTL2[1] = CreatePTE(AllocPage64(), TempPTL12);
	TPTL12[0] = (long)PD + 7;
	TPTL12[1] = TPTL4[0];
	TPTL12[2] = TPTL3[0];
	TPTL12[3] = TPTL2[0];
	TPTL12[4] = TPTL2[1];
	TPTL12[0x100] = CreatePTE(AllocPage64(), TempUserCode);
	TPTL12[0x110] = CreatePTE(AllocPage64(), TempUserData);
	return PD;
}

/*
====================================================
Create a Page Table Entry in the current Page Table
Zero fills the page
====================================================
*/
long CreatePTE(void * pAddress, long lAddress)
{
	long * PTableL11 = (long *)PageTableL11; 
	PTableL11[lAddress >> 12] = (long)pAddress | 7;
	char * c = (char *)lAddress;
	int count;
	for (count = 0; count < PageSize; count++) c[count] = 0;
	return (long)pAddress | 7;
}

/*
=========================================
Allocates one page of memory. 
Returns a pointer to the allocated page.
=========================================
*/
void * AllocPage64()
{
	long i = 0;

	while (PMap[i] != 0) i++;
	PMap[i] = 1;
	i = i << 12;
	nPagesFree--;
	return (void *) i;
}

/*
=========================================================================================
Searches the linked list pointed to by list for a block of memory of size sizeRequested
Allocates the memory and returns its address in RAX
=========================================================================================
*/
void * AllocMem(long sizeRequested, struct MemStruct * list)
{
	sizeRequested += sizeof(struct MemStruct);
	while (list->size < sizeRequested) list = list->next;
	if (list->size <= sizeRequested + 0x10)
	{
		list->size = 0;
	}
	else
	{
		void * temp = (void *) list;
		temp += sizeRequested;
		((struct MemStruct *)temp)->next = list->next;
		list->next = (struct MemStruct *) temp;
		list->next->size = list->size - sizeRequested;
		list->size = 0;
	}
	return list + 1;
}

/*
==================================================
Deallocate the memory at location list.
This will deallocate both user and kernel memory
==================================================
*/
void DeallocMem(void * list)
{
	struct MemStruct * l = (struct MemStruct *)list;
	l--;
	l->size = (void *)l->next - (void *)l;
}

/*
===============================================================================
Allocate some kernel memory from the heap. sizeRequested = amount to allocate
Returns in RAX address of allocated memory.
===============================================================================
*/
void * AllocKMem(long sizeRequested)
{
	void * temp = 0;
	while (temp == 0)
	{
		temp = AllocMem(sizeRequested, firstFreeKMem);
		if (temp == 0)
		{
			firstFreeKMem = (struct MemStruct*)(nextKPage << 12);
			CreatePTE(AllocPage64(), nextKPage);
			nextKPage++;
			struct MemStruct * tempmem = firstFreeKMem;
			while (tempmem->next != 0) tempmem = tempmem->next;
			tempmem->size += PageSize;
		}
	}
	return temp;
}

