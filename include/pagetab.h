#ifndef PAGETAB_H
#define PAGETAB_H

#define P	1		// Present(1)
#define RW	2		// Read(0)/Write(1)
#define US	4		// User(1)/Supervisor(0)
#define PWT	8		// Page-Level Writethrough
#define PCD 16		// Page-Level Cache Disable
#define A	32		// Accessed(1)
#define D	64 	// Dirty(1)
#define PAT 128	// Page Attribute Table (or sometimes Page Size, but always 0 for our purposes
#define G	256	// Global

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

void *AllocPage(unsigned short int pid);
long CreatePTE(void *pAddress, long lAddress, unsigned short pid, unsigned char global);
long AllocAndCreatePTE(long lAddress, unsigned short pid, unsigned char global);
long CreatePTEWithPT(struct PML4 *pml4, void *pAddress, long lAddress,
		     unsigned short pid, unsigned char global);
void *VCreatePageDir(unsigned short pid, unsigned short parentPid);

#endif
