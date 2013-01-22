#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kstructs.h"
#include "filesystem.h"

void *sys_AllocMem(long sizeRequested);
void *sys_AllocSharedMem(long sizeRequested);
void *sys_DeallocMem(void *memory);
void sys_ReceiveMessage(long port, struct Message *msg);
void sys_SendMessage(long port, struct Message *msg);
void sys_SendReceive(long port, struct Message *msg);
long sys_time();
long sys_GetTicks();
long sys_GetCurrentConsole();

#endif
