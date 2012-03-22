#include "memory.h"
#include "syscalls.h"

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
	sys_Exit();
}
