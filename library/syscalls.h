#ifndef SYSCALLS_H
#define SYSCALLS_H

void * sys_AllocMem(long sizeRequested);
void * sys_AllocSharedMem(long sizeRequested);
void * sys_DeallocMem(void * memory);
void sys_CreateTask(char * name, char * environment);
void sys_CreateKTask(void * code);
void sys_KillTask(void);
void sys_SendMessage(long port, struct Message * msg);
void sys_SendReceive(long port, struct Message * msg);
void sys_Sleep(int interval);
void * sys_GetCommandLine();

#endif
