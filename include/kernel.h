#ifndef KERNEL_H
#define KERNEL_H

#include "kstructs.h"

void SendMessage(struct MessagePort *MP, struct Message *Msg);
void ReceiveMessage(struct MessagePort *MP, struct Message *Msg);
void SendReceiveMessage(struct MessagePort *MP, struct Message *Msg);
struct MessagePort * AllocMessagePort();
void ReadSector(unsigned char * Buffer, unsigned int SectorNo);
void WriteSector(unsigned char * Buffer, unsigned int SectorNo);
void WaitForInt(long interrupt);
long ReadFromFile(struct FCB *fHandle, char *buffer, long noBytes);
void copyMem(unsigned char *source, unsigned char *dest, long size);
void copyString(unsigned char *source, unsigned char *destination);
void BlockTask(struct Task *task);
void UnBlockTask(struct Task *task);
void moveTaskToEndOfQueue();
void SetSem(long * semaphore);
void ClearSem(long * semaphore);
void KWriteString(char *str, int row, int col);
struct Task * PidToTask(long pid);

#endif
