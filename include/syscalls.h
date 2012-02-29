#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kstructs.h"

void * sys_AllocMem(long sizeRequested);
void * sys_AllocSharedMem(long sizeRequested);
void * sys_DeallocMem(void * memory);
void sys_Exit(void);
void sys_ReceiveMessage(long port, struct Message *msg);
void sys_SendMessage(long port, struct Message * msg);
void sys_SendReceive(long port, struct Message * msg);
void sys_Sleep(int interval);
long sys_GetTicks();
long sys_GetCurrentConsole();
long sys_SetCurrentDirectory(long directory);
struct FCB * Sys_Open(char * filename);
struct FCB * Sys_close(char * filename);
long Sys_Execve(char * name, char * environment);
void Sys_Wait(unsigned short pid);

#endif
