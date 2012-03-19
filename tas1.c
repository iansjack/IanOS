#include "memory.h"
#include "syscalls.h"

extern void kbTaskCode(void);
extern void consoleTaskCode(void);
extern void fsTaskCode(void);
extern void dummyTask(void);
extern void monitorTaskCode(void);

// This task is just here to start the ball rolling

void tas1(void)
{
	// Give tasks time to set themselves up
	Sys_Nanosleep(10);

	long pid = Sys_Fork();
	if (!pid)
		Sys_Execve("TASK1", "TASK1 0");
	pid = Sys_Fork();
	if (!pid)
		Sys_Execve("TASK1", "TASK1 1");
	pid = Sys_Fork();
/*	if (!pid)
		Sys_Execve("TASK1", "TASK1 2");
	pid = Sys_Fork();
	if (!pid)
		Sys_Execve("TASK1", "TASK1 3");
*/ sys_Exit();
}
