#include "cmemory.h"
#include "library/syscalls.h"

extern struct Task * currentTask;
extern void kbTaskCode(void);
extern void consoleTaskCode(void);

void tas1(void)
{
	long * l = (long *) UserData;
	l[1] = 0xFF8;
	sys_CreateKTask(kbTaskCode);
	sys_Sleep(10);
	sys_CreateKTask(consoleTaskCode);
	sys_Sleep(10);
	sys_CreateTask("TASK1.BIN");
	sys_KillTask();
}
