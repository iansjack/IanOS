#ifndef MEMORY_H
#define MEMORY_H

#include "kstructs.h"
#include "pagetab.h"

//=============
// Memory Map
//=============
#define GDT             0x0000000000000000L
#define IDT             0x0000000000000800L
#define OSCode          0x0000000000001000L
#define OSData          0x0000000000010000L
#define StaticPort      0x000000000006F000L
#define PageMap         0x000000000007F000L
#define TaskStruct      0x0000000000080000L
#define TempUserCode    0x0000000000095000L
#define TempUserData    0x0000000000096000L
#define TempKStack      0x0000000000097000L
#define TempUStack      0x0000000000098000L
#define PageTableL4     0x0000000000200000L     // we will create the tables and fill in
#define PageTableL3     0x0000000000201000L     // the physical addresses
#define PageTableL2     0x0000000000202000L     //
#define PageTableL11    0x0000000000203000L     // This contains the entries for 0x0000000000000000 - 0x00000000001FFFFF
#define PageTableL12    0x0000000000204000L     // This contains the entries for 0x0000000000200000 - 0x00000000003FFFFF
#define UserCode        0x0000000000300000L
#define UserData        0x0000000000310000L
#define KernelStack     0x00000000003FC000L
#define UserStack       0x00000000003FE000L

// Free			0x0000000000400000 - 0x0000003FFFFFFFFF
// Virtual     0x0000003FFFFFFFFF - 0x000000FFFFFFFFFF   // Complete map of physical addresses

#define PageSize       0x1000
#define KbdPort        0x6F010L
#define ConsolePort    0x6F020L
#define FSPort         0x6F030L

#define OsCodeSeg      0x8
#define OsDataSeg      0x10
#define code64         0x18
#define data64         0x20
#define udata64        0x28
#define user64         0x30
#define tssd64         0x38

#define tsslength      0x80

void *AllocMem(long sizeRequested, struct MemStruct *list);
void *AllocKMem(long sizeRequested);
void *AllocUMem(long sizeRequested);
void *AllocSharedMem(long sizeRequested);
void DeallocMem(void *list);
void DeallocSharedMem(long pid);
void DeallocKMem(long pid);

#endif
