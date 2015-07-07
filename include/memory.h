#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include "kstructs.h"

//=============
// Memory Map
//=============
#define GDT             0x0000000000000000L
#define IDT             0x0000000000000800L
#define OSCode          0x0000000000010000L
#define OSData          0x0000000000020000L
#define OSHeap			0x0000000000022000L
#define TempUserCode    0x00000000000FD000L
#define TempUserData    0x00000000000FE000L
#define TempUStack      0x00000000000FF000L
#define PageMap         0x0000000000100000L
#define UserCode        0x0000000004000000L
#define UserData        0x0000000010000000L
#define UserStack     	0x00000000ffffe000L
#define KernelStack     0x00000000fffff000L
#define SharedMemory	0x000000000f000000L
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

void *AllocMem(size_t sizeRequested, struct MemStruct *list);
void *AllocKMem(size_t sizeRequested);
void *AllocUMem(size_t sizeRequested);
void *AllocSharedMem(size_t sizeRequested);
void DeallocMem(void *list);
void DeallocSharedMem(long pid);
void DeallocKMem(long pid);

#endif
