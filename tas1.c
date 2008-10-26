#include "cmemory.h"
#include "library/syscalls.h"

extern struct Task * currentTask;

void tas1(void)
{
	long * l = (long *) UserData;
	l[1] = 0xFF8;
	sys_CreateTask("TASK1   BIN");
	sys_KillTask();
	//sys_CreateTask("TASK2   BIN");
	//while (1)
	//{
	//	WriteDouble(GetTicks(), 20, 60);
	//}
}
