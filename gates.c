#include "cmemory.h"

	extern intr;
	extern gpf;
	extern pf;
	extern SwitchTasks;
	extern SpecificSwitchTasks;
	extern TimerInt;
	extern KbInt;
	extern HdInt;

struct Gate
{
	unsigned short offsetlo; // 	: 16;
	unsigned short segselect; // 	: 16;
	unsigned ist 			: 3;
	unsigned			: 1;
	unsigned			: 1;
	unsigned			: 3;
	unsigned type			: 4;
	unsigned			: 1;
	unsigned dpl			: 2;
	unsigned p			: 1;
	unsigned short offsetmid; //	: 16;
	unsigned int offsethi; //	: 32;
	unsigned int reserved; //	: 32;
};

struct TSSDescriptor
{
	unsigned short limitlo;
	unsigned short baselo;
	unsigned basemid1		: 8;
	unsigned type			: 4;
	unsigned			: 1;
	unsigned dpl			: 2;
	unsigned p			: 1;
	unsigned limithi		: 4;
	unsigned avl			: 1;
	unsigned			: 1;
	unsigned			: 1;
	unsigned g			: 1;
	unsigned basemid2		: 8;
	unsigned int basehi;
	unsigned			: 32;
};

void CreateTrapGate(short selector, long offset, long intNo)
{
	struct Gate * theGate = (struct Gate *)IDT + intNo;
	theGate-> offsetlo = (unsigned short)(offset & 0xFFFF);
	theGate->segselect = selector;
	theGate->ist = 0;
	theGate->type = 0xF;
	theGate->p = 1;
	theGate->offsetmid = (unsigned short)((offset >>16) & 0xFFFF);
	theGate->offsethi = (unsigned int)((offset >> 32) & 0xFFFFFFFF);
	theGate->reserved = 0;
}

void CreateIntGate(short selector, long offset, long intNo)
{
	struct Gate * theGate = (struct Gate *)IDT + intNo;
	theGate-> offsetlo = (unsigned short)(offset & 0xFFFF);
	theGate->segselect = selector;
	theGate->ist = 0;
	theGate->type = 0xE;
	theGate->p = 1;
	theGate->offsetmid = (unsigned short)((offset >>16) & 0xFFFF);
	theGate->offsethi = (unsigned int)((offset >> 32) & 0xFFFFFFFF);
	theGate->reserved = 0;
}

void InitIDT(void)
{
	int i;
	for (i = 0; i < 48; i++)
		CreateTrapGate(code64, (long)&intr, i);
	CreateTrapGate(code64, (long)&gpf, 13);
	CreateTrapGate(code64, (long)&pf, 14);
	CreateTrapGate(code64, (long)&SwitchTasks, 20);
	CreateTrapGate(code64, (long)&SpecificSwitchTasks, 22);
	CreateTrapGate(code64, (long)&TimerInt, 32);
	CreateIntGate(code64, (long)&KbInt, 33);
	CreateIntGate(code64, (long)&HdInt, 46);
}

void CreateTssDesc(long base, int selector)
{
	long * l;
	struct TSSDescriptor * desc = (struct TSSDescriptor *)(GDT + selector);
	l = (long *)desc;
	l[0] = 0;
	l[1] = 0;
	desc->limitlo = tsslength;
	desc->limithi = 0;
	desc->type = 9;
	desc->dpl = 0;
	desc->p = 1;
	desc->baselo = (unsigned short)(base & 0xFFFF);
	desc->basemid1 = (unsigned char)((base >> 16) & 0xFF);
	desc->basemid2 = (unsigned char)((base >>20) & 0xFF);
	desc->basehi = (unsigned int)((base >> 24) & 0xFFFFFFFF);
}
