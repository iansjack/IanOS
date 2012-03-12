#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kstructs.h"
#include "filesystem.h"

void * sys_AllocMem(long sizeRequested);
void * sys_AllocSharedMem(long sizeRequested);
void * sys_DeallocMem(void * memory);
void sys_Exit(void);
void sys_ReceiveMessage(long port, struct Message *msg);
void sys_SendMessage(long port, struct Message * msg);
void sys_SendReceive(long port, struct Message * msg);
void Sys_Nanoleep(int interval);
long sys_GetTicks();
unsigned char *Sys_Getcwd();
long sys_GetCurrentConsole();
long Sys_Chdir(unsigned char *directory);
FD Sys_Open(unsigned char * filename);
int Sys_Close(FD fileDescriptor);
long Sys_Execve(unsigned char * name, char * environment);
void Sys_Wait(unsigned short pid);
int Sys_Stat(FD fileDescriptor, struct FileInfo *info);
long Sys_Read(FD fileDescriptor, unsigned char *buffer, long noBytes);
long Sys_Write(FD fileDescriptor, unsigned char *buffer, long noBytes);
FD Sys_Create(unsigned char *name);
int Sys_Unlink(unsigned char *name);

#endif
