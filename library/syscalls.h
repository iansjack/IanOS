#ifndef SYSCALLS_H
#define SYSCALLS_H

void * sys_AllocMem(long sizeRequested);
void * sys_DeallocMem(void * memory);
void sys_CreateTask(char * name);
void sys_KillTask(void);

#endif
