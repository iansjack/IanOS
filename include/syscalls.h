#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kstructs.h"
#include "filesystem.h"

typedef unsigned long l_Address;

void *sys_AllocMem(long sizeRequested);
void *sys_AllocSharedMem(long sizeRequested);
void *sys_DeallocMem(void *memory);
void sys_receivemessage(struct MessagePort *port, struct Message *msg);
void sys_sendmessage(struct MessagePort *port, struct Message *msg);
void sys_sendreceive(struct MessagePort *port, struct Message *msg);
long sys_time();
long sys_GetTicks();
long sys_GetCurrentConsole();
struct MessagePort *sys_getnetport();
struct MessagePort *sys_allocmessageport();
l_Address Alloc_Page(unsigned char);
void Alloc_Shared_Page(unsigned short pid, void * lAddress1, void *lAddress2);

#endif
