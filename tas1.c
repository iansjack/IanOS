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
	//asm("mov $10, %rdi");
	//asm("mov $8, %r9");
	//asm("syscall");
	sys_Sleep(10);
	sys_CreateKTask(consoleTaskCode);
	sys_CreateTask("TASK1.BIN");
	sys_KillTask();
}
