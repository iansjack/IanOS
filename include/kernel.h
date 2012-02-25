#ifndef KERNEL_H
#define KERNEL_H

#include "kstructs.h"

void SendMessage(struct MessagePort *MP, struct Message *Msg);
void ReceiveMessage(struct MessagePort *MP, struct Message *Msg);
void SendReceiveMessage(struct MessagePort *MP, struct Message *Msg);
void ReadSector(unsigned char * Buffer, unsigned int SectorNo);
void WriteSector(unsigned char * Buffer, unsigned int SectorNo);
void WaitForInt(long interrupt);
long ReadFromFile(struct FCB *fHandle, char *buffer, long noBytes);
void copyMem(unsigned char source[], unsigned char dest[], long size);
void BlockTask(struct Task *task);
void UnBlockTask(struct Task *task);
void moveTaskToEndOfQueue();
//void RemoveFromQ(struct Task *task, struct Task **QHead, struct Task **QTail);
//void AddToQ(struct Task *task, struct Task **QHead, struct Task **QTail);
void SetSem(long * semaphore);
void ClearSem(long * semaphore);
void KWriteString(char *str, int row, int col);
struct Task * PidToTask(long pid);
//long strncmp(char * s1, char * s2, long length);

#endif
