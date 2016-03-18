#include <kernel.h>
#include <mp.h>
#include <transfer.h>

unsigned char *PMap;
struct transfer *t;

void SetBit32(int count)
{
	int i = count / 8;
	int j = count % 8;
	PMap[i] |= 1 << j;
}

void ClearBit32(int count)
{
	int i = count / 8;
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
	char * source = (char *) 0x800;

	t = (struct transfer *) 0x910;
	long *memSize;
	memSize = (long *) 0x900;
	t->nPages = *memSize / 4;

	PMap = (unsigned char *) PageMap;

	long count;
	t->nPagesFree = t->nPages;

	for (count = 0; count < t->nPages / 8; count++)
		PMap[count] = 0;

	// Mark first MB of memory and all OS code and data as used
	for (count = 0; count <= 0x200; count++)
	{
		SetBit32(count);
		t->nPagesFree--;
	}

//	for (count = 0x180; count <= 0x1ff; count++)
//	{
//		SetBit32(count);
//		t->nPagesFree--;
//	}
	// PageMap
//	for (count = 0x100; count < 0x100 + (2 * t->nPages / (long) PageSize);
//			count++)
//	{
//		SetBit32(count);
//		t->nPagesFree--;
//	}

	t->firstFreePage = count + 1;

	// Zero unused pages
	for (count = 0; count < t->nPages; count++)
		if (!GetBit32)
			for (i = 0; i < PageSize; i++)
				*((char *) (count * PageSize + i)) = 0;
}

//============================================
// Allocate a page of memory and zero fill it
//============================================
void *AllocPage32(unsigned short int PID)
{
	unsigned short int *PMap = (unsigned short int *) PageMap;
	int count;

	unsigned char *mem = (unsigned char *) ((long) (t->firstFreePage << 12));
	SetBit32(t->firstFreePage);
	while (GetBit32(++(t->firstFreePage)))
		;
	t->nPagesFree--;
	for (count = 0; count < PageSize; count++)
		mem[count] = 0;
	return (void *) mem;
}

