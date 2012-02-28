#include "memory.h"
#include "syscalls.h"

extern void kbTaskCode(void);
extern void consoleTaskCode(void);
extern void fsTaskCode(void);
extern void dummyTask(void);
extern void monitorTaskCode(void);

void tas1(void)
{
   sys_CreateLPTask(dummyTask);
   sys_CreateKTask(kbTaskCode);
   sys_CreateKTask(consoleTaskCode);
   sys_CreateKTask(fsTaskCode);
	long pid = Sys_Fork();
	if (pid)
	{
		sys_WriteString("This is the forking process", 20, 0);
	}
	else
	{
		sys_WriteString("This is the forked process", 21, 0);
   		sys_Sleep(100);
		Sys_Execve("TASK1", 0);
	}
   sys_Exit();
}
