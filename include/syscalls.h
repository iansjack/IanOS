#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kstructs.h"
#include "filesystem.h"

void *sys_AllocMem(long sizeRequested);
void *sys_AllocSharedMem(long sizeRequested);
void *sys_DeallocMem(void *memory);
void exit(void);
void sys_ReceiveMessage(long port, struct Message *msg);
void sys_SendMessage(long port, struct Message *msg);
void sys_SendReceive(long port, struct Message *msg);
void nanoleep(int interval);
long sys_GetTicks();
unsigned char *getcwd();
long sys_GetCurrentConsole();
long chdir(unsigned char *directory);
FD open(unsigned char *filename);
int close(FD fileDescriptor);
long execve(unsigned char *name, char *environment);
void wait(unsigned short pid);
int stat(FD fileDescriptor, struct FileInfo *info);	// Wrong - should take filename as parameter!!!
int fstat(FD fileDescriptor, struct FileInfo *info);
long read(FD fileDescriptor, unsigned char *buffer, long noBytes);
long write(FD fileDescriptor, unsigned char *buffer, long noBytes);
FD creat(unsigned char *name);
int unlink(unsigned char *name);
long mkdir(unsigned char *name);
int lseek(FD fDescriptor, int offset, int whence);
int fork();
void *malloc(int size);
void free(void *ptr);

#endif
