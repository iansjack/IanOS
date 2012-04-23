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
#define OSHeap			0x0000000000011000L
#define TempUserCode    0x0000000000095000L
#define TempUserData    0x0000000000096000L
#define TempUStack      0x0000000000098000L
#define PageMap         0x0000000000100000L
#define UserCode        0x0000000000400000L
#define UserData        0x0000000000600000L
#define KernelStack     0x00000000009ff000L
#define UserStack       0x0000000000bff000L
#define VAddr			0x0000008000000000L

// Free                 0x0000000000C00000 - 0x0000007FFFFFFFFF

#define PageSize		0x1000

#define OsCodeSeg		0x8
#define OsDataSeg		0x10
#define code64			0x18
#define data64			0x20
#define udata64			0x28
#define user64			0x30
#define tssd64			0x38

#define tsslength		0x80

void *AllocMem(long sizeRequested, struct MemStruct *list);
void *AllocKMem(long sizeRequested);
void *AllocUMem(long sizeRequested);
void *AllocSharedMem(long sizeRequested);
void DeallocMem(void *list);
void DeallocSharedMem(long pid);
void DeallocKMem(long pid);

#endif
