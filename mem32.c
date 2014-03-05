#include <kernel.h>

extern long long nPagesFree;
extern long long nPages;
extern long long firstFreePage;
extern char *mMap;
extern struct MemoryMap mmap[16];	// This doesn't cut it, but OK for testing.

unsigned char *PMap;

void SetBit32(int count)
{
	int i = count / 8;
	int j = count % 8;
	PMap[i] |= 1 << j;
}

void ClearBit32(int count)
{
	int i = count /8;
	int j = count % 8;
	PMap[i] &= ~(1 << j);
}

int GetBit32(int count)
{
	int i = count / 8;
	int j = count % 8;
	if (PMap[i] & (1 << j))
		return 1;
	else
		return 0;
}

//========================================
// Find out how much memory is present.
// Initialize some variables.
// Create PageMap.
//========================================
void InitMemManagement()
{
	int i = 0;
	char * source = (char *)0x800;
	for (i = 0; i < 192; i++) mMap[i] = source[i];

	long *memSize;
	memSize = (long *)0x900;
	nPages = *memSize / 4;

	PMap = (unsigned char *) PageMap;

	long count;
	nPagesFree = nPages;

	for (count = 0; count < nPages / 8; count++)
		PMap[count] = 0;

	// GDT and IDT
	SetBit32(0);
	nPagesFree--;

	// Kernel Memory
	for (count = 1; count < 0x23; count++)
	{
		SetBit32(count);
		nPagesFree--;
	}

	firstFreePage = 0x23;

	// EBDA
	SetBit32(0x9E); SetBit32(0x9F);
	nPagesFree -= 2;

	// Ports, ROM, VideoMem, etc.
	for (count = 0xA0; count < 0x100; count++)
	{
		SetBit32(count);
		nPagesFree--;
	}

	// PageMap
	for (count = 0x100; count < 0x100 + (2 * (long) nPages / (long) PageSize);
			count++)
	{
		SetBit32(count);
		nPagesFree--;
	}

	// Zero unused pages
	for (count = 0; count < nPages; count++)
		if (!GetBit32)
			for (i=0; i < PageSize; i++)
				*((char *)(count * PageSize + i)) = 0;
}

//============================================
// Allocate a page of memory and zero fill it
//============================================
void *AllocPage32(unsigned short int PID)
{
	unsigned short int *PMap = (unsigned short int *) PageMap;
	int count;

	unsigned char *mem = (unsigned char *) ((long) (firstFreePage << 12));
	SetBit32(firstFreePage);
	while (GetBit32(++firstFreePage))
		;
	nPagesFree--;
	for (count = 0; count < PageSize; count++)
		mem[count] = 0;
	return (void *) mem;
}
