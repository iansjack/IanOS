#ifndef KERNEL_H
#define KERNEL_H

// #define DEBUG

#include <kstructs.h>
#include <memory.h>
#include <pagetab.h>
#include <lib.h>
#include <tasklist.h>

void SendMessage(struct MessagePort *MP, struct Message *Msg);
void ReceiveMessage(struct MessagePort *MP, struct Message *Msg);
void SendReceiveMessage(struct MessagePort *MP, struct Message *Msg);
struct MessagePort *AllocMessagePort();
unsigned char *ReadSector(unsigned int SectorNo);
void WriteSector(unsigned int SectorNo);
void WaitForInt(long interrupt);
long ReadFromFile(struct FCB *fHandle, char *buffer, long noBytes);
void copyMem(unsigned char *source, unsigned char *dest, long size);
void copyString(unsigned char *source, unsigned char *destination);
void BlockTask(struct Task *task);
void UnBlockTask(struct Task *task);
void moveTaskToEndOfQueue();
void SetSem(long *semaphore);
void ClearSem(long *semaphore);
void KWriteString(char *str, int row, int col);
struct Task *PidToTask(long pid);
unsigned char *NameToFullPath(unsigned char *name);
int kprintf(int row, int column, unsigned char *s, ...);
long strlen(unsigned char *string);
long strcmp(unsigned char *s1, unsigned char *s2);

#define ALLOCMSG (struct Message *)AllocKMem(sizeof(struct Message))
#define ASSERT(expr) if (!expr){kprintf(24, 1, "Assertion failed - System halted.");asm("cli");asm("hlt");}

#endif
