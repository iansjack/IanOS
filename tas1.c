#include "cmemory.h"
#include "library/syscalls.h"

extern struct Task * currentTask;
extern void kbTaskCode(void);
extern void consoleTaskCode(void);
extern void fsTaskCode(void);

void tas1(void)
{
	long * l = (long *) UserData;
	l[1] = 0xFF8;
	sys_CreateKTask(kbTaskCode);
	sys_Sleep(5);
	sys_CreateKTask(consoleTaskCode);
	sys_Sleep(5);
	sys_CreateKTask(fsTaskCode);
	sys_Sleep(5);
	sys_CreateTask("TASK1.BIN");
	//sys_KillTask();
	while(1)
	{
	    //sys_Sleep(120);
	}
}
