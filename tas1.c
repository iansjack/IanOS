#include "memory.h"
#include "syscalls.h"

// This task is just here to start the ball rolling

void tas1(void)
{
	// Give tasks time to set themselves up
	nanosleep(10);

	long pid = fork();
	if (!pid)
		execve("TASK1", "TASK1 0");
	pid = fork();
	if (!pid)
		execve("TASK1", "TASK1 1");
//	pid = Sys_Fork();
	exit();
}
