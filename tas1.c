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
   sys_Sleep(40);
   sys_CreateKTask(kbTaskCode);
   sys_Sleep(40);
   sys_CreateKTask(consoleTaskCode);
   sys_Sleep(40);
   sys_CreateKTask(fsTaskCode);
   sys_Sleep(40);
	long pid = Sys_Fork();
	if (pid)
	{
   		// sys_CreateTask("TASK1", "", 0, 0);
		sys_WriteString("This is the forking process", 20, 0);
   		sys_Sleep(40);
	}
	else
	{
   		sys_CreateTask("TASK1", "", 0, 1);
		sys_WriteString("This is the forked process", 21, 0);
   		sys_Sleep(40);
	}
//   sys_CreateTask("TASK1", "", 0, 2);
//   sys_Sleep(40);
//   sys_CreateKTask(monitorTaskCode);
//   sys_Sleep(4);
   sys_Exit();
}
