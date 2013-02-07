#include <kernel.h>

extern long long memorySemaphore;
extern long long nPagesFree;
extern long long nPages;
extern long long firstFreePage;

//========================================
// Find out how much memory is present.
// Initialize some variables.
// Create PageMap.
//========================================
void InitMemManagement()
{
	unsigned short int *PMap = (unsigned short int *) PageMap;

	memorySemaphore = 0;
	nPagesFree = 256;

	// Find number of free pages by writing a pattern to memory and seeing if it reads back OK
	// We start at 1Mb and go up in 1Mb increments. Each Mb is 256 pages.
	long *mempos = (long *) 0x100000;
	long testpattern = 0x6d72646c;
	while (1)
	{
		*mempos = testpattern;
		if (*mempos != testpattern)
			break;
		mempos += 0x100000 / (sizeof *mempos);
		nPagesFree += 256;
	}
	nPages = nPagesFree;

	int count;
	for (count = 0; count < nPages; count++)
		PMap[count] = 0;

	// GDT and IDT
	PMap[0] = 1;
	nPagesFree--;

	// Kernel Memory
	for (count = 1; count < 0x13; count++)
	{
		PMap[count] = 1;
		nPagesFree--;
	}

	firstFreePage = 0x13;

	// EBDA
	PMap[0x9E] = PMap[0x9F] = 1;
	nPagesFree -= 2;

	// Ports, ROM, VideoMem, etc.
	for (count = 0xA0; count < 0x100; count++)
	{
		PMap[count] = 1;
		nPagesFree--;
	}

	// PageMap
	for (count = 0x100; count < 0x100 + (2 * (long) nPages / (long) PageSize);
			count++)
	{
		PMap[count] = 1;
		nPagesFree--;
	}
}

void ZeroMem()
{
	long count;

}
//============================================
// Allocate a page of memory and zero fill it
//============================================
void *AllocPage32(unsigned short int PID)
{
	unsigned short int *PMap = (unsigned short int *) PageMap;
	int count;

	unsigned char *mem = (unsigned char *) ((long) (firstFreePage << 12));
	PMap[firstFreePage] = PID;
	while (PMap[++firstFreePage])
		;
	nPagesFree--;
	for (count = 0; count < PageSize; count++)
		mem[count] = 0;
	return (void *) mem;
}
