#ifndef PAGETAB_H
#define PAGETAB_H

#define P		1				// Present(1)
#define RW		2				// Read(0)/Write(1)
#define US		4				// User(1)/Supervisor(0)
#define PWT		8				// Page-Level Writethrough
#define PCD 	16				// Page-Level Cache Disable
#define A		32				// Accessed(1)
#define D		64 				// Dirty(1)
#define PAT 	128				// Page Attribute Table (or sometimes Page Size, but always 0 for our purposes
#define G		256				// Global
#define	NX32	0x80000000
#define NX		0x8000000000000	// No execute

#ifdef CODE_32
struct PML4e {
	long Lo:32;
	long Hi:32;
};

struct PDPE {
	long Lo:32;
	long Hi:32;
};

struct PDE {
	long Lo:32;
	long Hi:32;
};

struct PTE {
	long Lo:32;
	long Hi:32;
};
#else
struct PML4e {
	long value;
};

struct PDPE {
	long value;
};

struct PDE {
	long value;
};

struct PTE {
	long value;
};
#endif

struct PML4 {
	struct PML4e entries[64];
};

struct PDP {
	struct PDPE entries[64];
};

struct PD {
	struct PDE entries[64];
};

struct PT {
	struct PTE entries[64];
};

p_Address AllocPage(unsigned short int pid);
p_Address CreatePTE(p_Address pAddress, l_Address lAddress, unsigned short pid, short flags);
p_Address AllocAndCreatePTE(l_Address lAddress, unsigned short pid, short flags);
p_Address CreatePTEWithPT(struct PML4 *pml4, p_Address pAddress, l_Address lAddress,
		     unsigned short pid, short flags);
p_Address VCreatePageDir(unsigned short pid, unsigned short parentPid);
void ClearUserMemory(void);
void AllocateRange(l_Address lAddress, long size, unsigned short pid);
struct PT *GetPT(struct PML4 *, l_Address, unsigned short);
p_Address checkPTEWithPT(struct PML4 *, l_Address);
p_Address checkPTE(l_Address lAddress);

#define VIRT(type, name) ((struct type *) ((long) name + VAddr))

#endif
