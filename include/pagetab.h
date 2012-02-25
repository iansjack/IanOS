#ifndef PAGETAB_H
#define PAGETAB_H

#define P 1
#define RW 2
#define US 4
#define PWT 8
#define PCD 16
#define A 32
#define D 64
#define PAT 128
#define G 256

#ifdef CODE_32
struct PML4e
{
   long Lo :32;
   long Hi :32;
};

struct PDPE
{
   long Lo :32;
   long Hi :32;
};

struct PDE
{
   long Lo :32;
   long Hi :32;
};

struct PTE
{
   long Lo :32;
   long Hi :32;
};
#else
struct PML4e
{
   long value;
};

struct PDPE
{
   long value;
};

struct PDE
{
   long value;
};

struct PTE
{
   long value;
};
#endif

struct PML4
{
   struct PML4e entries[64];
};

struct PDP
{
   struct PDPE entries[64];
};

struct PD
{
   struct PDE entries[64];
};

struct PT
{
   struct PTE entries[64];
};

void *
AllocPage(unsigned short int pid);
long
CreatePTE(void *pAddress, long lAddress);
void *
VCreatePageDir(unsigned short pid);

#endif
