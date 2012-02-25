#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kstructs.h"

void * sys_AllocMem(long sizeRequested);
void * sys_AllocSharedMem(long sizeRequested);
struct MessagePort * sys_AllocMessagePort();
void * sys_DeallocMem(void * memory);
void sys_CreateTask(char * name, char * environment, struct MessagePort * parentPort, long console);
void sys_CreateKTask(void * code);
void sys_CreateLPTask(void * code);
void sys_KillTask(void);
void sys_ReceiveMessage(long port, struct Message *msg);
void sys_SendMessage(long port, struct Message * msg);
void sys_SendReceive(long port, struct Message * msg);
void sys_Sleep(int interval);
void * sys_GetCommandLine();
void sys_WriteDouble(int n, int x, int y);
void sys_WriteString(unsigned char * string, int x, int y);
long sys_GetTicks();
long sys_GetCurrentConsole();
long sys_SetCurrentDirectory(long directory);
struct FCB * Sys_Open(char * filename);
struct FCB * Sys_close(char * filename);

#endif
