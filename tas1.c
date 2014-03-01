#include <unistd.h>

void nanosleep(long);	// Defined in scalls.s

// This task is just here to start the ball rolling

void tas1(void)
{
	long pid;
	// Give tasks time to set themselves up
	nanosleep(10);

	pid = fork();
	if (!pid)
		execve((const char *) "sh", (char * const *) "sh", /*NULL*/ 0);

	exit(0);
}
