
#include <memory.h>

extern spr;
extern intr;
extern gpf;
extern pf;
extern SwitchTasks;
extern SpecificSwitchTasks;
extern TimerInt;
extern KbInt;
// extern NicInt;
extern HdInt;
extern div0;
extern i1;
extern i2;
extern i3;
extern i4;
extern i5;
extern i6;
extern i7;
extern df;
extern i9;
extern ia;
extern ib;
extern ic;

struct Gate {
	unsigned short offsetlo;	//   : 16;
	unsigned short segselect;	//   : 16;
	unsigned ist:3;
	unsigned r1:1;
	unsigned r2:1;
	unsigned r3:3;
	unsigned type:4;
	unsigned r4:1;
	unsigned dpl:2;
	unsigned p:1;
	unsigned short offsetmid;	//    : 16;
	unsigned int offsethi;	//       : 32;
	unsigned int reserved;	//       : 32;
}__attribute__((packed));

struct TSSDescriptor {
	unsigned short limitlo;
	unsigned short baselo;
	unsigned basemid1:8;
	unsigned type:4;
	unsigned:1;
	unsigned dpl:2;
	unsigned p:1;
	unsigned limithi:4;
	unsigned avl:1;
	unsigned:1;
	unsigned:1;
	unsigned g:1;
	unsigned basemid2:8;
	unsigned int basehi;
	unsigned:32;
}__attribute__((packed));

void CreateTrapGate(unsigned short int selector, long offset, long intNo, unsigned char ist)
{
	struct Gate *theGate = (struct Gate *)IDT + intNo;

	theGate->r1 = 0;
	theGate->r2 = 0;
	theGate->r3 = 0;
	theGate->r4 = 0;
	theGate->offsetlo = (unsigned short)(offset & 0xFFFF);
	theGate->segselect = selector;
	theGate->ist = ist;
	theGate->type = 0xF;
	theGate->p = 1;
	theGate->offsetmid = (unsigned short)((offset >> 16) & 0xFFFF);
	theGate->offsethi = (unsigned int)((offset >> 32) & 0xFFFFFFFF);
	theGate->reserved = 0;
}

void CreateIntGate(unsigned short int selector, long offset, long intNo, unsigned char ist)
{
	struct Gate *theGate = (struct Gate *)IDT + intNo;

	theGate->r1 = 0;
	theGate->r2 = 0;
	theGate->r3 = 0;
	theGate->r4 = 0;
	theGate->offsetlo = (unsigned short)(offset & 0xFFFF);
	theGate->segselect = selector;
	theGate->ist = ist;
	theGate->type = 0xE;
	theGate->p = 1;
	theGate->offsetmid = (unsigned short)((offset >> 16) & 0xFFFF);
	theGate->offsethi = (unsigned int)((offset >> 32) & 0xFFFFFFFF);
	theGate->reserved = 0;
}

void InitIDT(void)
{
	int i;

	for (i = 0; i < 48; i++) {
		CreateTrapGate(code64, (long)&intr, i, 0);
	}
	CreateTrapGate(code64, (long)&div0, 0, 1);
	CreateTrapGate(code64, (long)&i1, 1, 1);
	CreateTrapGate(code64, (long)&i2, 2, 1);
	CreateTrapGate(code64, (long)&i3, 3, 1);
	CreateTrapGate(code64, (long)&i4, 4, 1);
	CreateTrapGate(code64, (long)&i5, 5, 1);
	CreateTrapGate(code64, (long)&i6, 6, 1);
	CreateTrapGate(code64, (long)&i7, 7, 1);
	CreateTrapGate(code64, (long)&df, 8, 1);
	CreateTrapGate(code64, (long)&i9, 9, 1);
	CreateTrapGate(code64, (long)&ia, 10, 1);
	CreateTrapGate(code64, (long)&ib, 11, 1);
	CreateTrapGate(code64, (long)&ic, 12, 1);
	CreateTrapGate(code64, (long)&gpf, 13, 1);
	CreateTrapGate(code64, (long)&pf, 14, 1);
	CreateTrapGate(code64, (long)&SwitchTasks, 20, 0);
	CreateTrapGate(code64, (long)&SpecificSwitchTasks, 22, 0);
	CreateTrapGate(code64, (long)&TimerInt, 32, 0);
	CreateIntGate(code64, (long)&KbInt, 33, 0);
	// CreateIntGate(code64, (long)&NicInt, 42, 0);
	// CreateIntGate(code64, (long)&NicInt, 43, 0);
	CreateIntGate(code64, (long)&HdInt, 46, 0);
	CreateIntGate(code64, (long)&spr, 39, 0);
	CreateIntGate(code64, (long)&spr, 47, 0);
}

void CreateTssDesc(long base, int selector)
{
	long *l;
	struct TSSDescriptor *desc = (struct TSSDescriptor *)(GDT + selector);

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
	desc->basemid2 = (unsigned char)((base >> 20) & 0xFF);
	desc->basehi = (unsigned int)((base >> 24) & 0xFFFFFFFF);
}
