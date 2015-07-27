#ifndef KERNEL_H
#define KERNEL_H

// #define DEBUG

#include "kstructs.h"
#include "memory.h"
#include "lib.h"
#include "tasklist.h"

void SendMessage(struct MessagePort *MP, struct Message *Msg);
void ReceiveMessage(struct MessagePort *MP, struct Message *Msg);
void SendReceiveMessage(struct MessagePort *MP, struct Message *Msg);
struct MessagePort *AllocMessagePort();
unsigned char *ReadSector(unsigned int SectorNo);
void WriteSector(unsigned int SectorNo);
void WaitForInt(long interrupt);
long ReadFromFile(struct FCB *fHandle, char *buffer, long noBytes);
long SeekFile(struct Message *FSMsg, struct FCB * fHandle, long offset, long whence);
//void copyMem(char *source, char *dest, size_t size);
void copyString(char *source, char *destination);
void BlockTask(struct Task *task);
void UnBlockTask(struct Task *task);
void moveTaskToEndOfQueue();
void SetSem(long *semaphore);
void ClearSem(long *semaphore);
void KWriteString(char *str, int row, int col);
struct Task *PidToTask(unsigned short pid);
char *NameToFullPath(char *name);
int kprintf(int row, int column, char *s, ...);
long strlen(char *string);
long strcmp(char *s1, char *s2);

#define ALLOCMSG (struct Message *)AllocKMem(sizeof(struct Message))
#define ASSERT(expr) if (!expr){kprintf(24, 1, "Assertion failed - System halted.");asm("cli");asm("hlt");}

#endif
