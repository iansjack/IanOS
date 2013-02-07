#include <unistd.h>

// This task is just here to start the ball rolling

void tas1(void)
{
	// Give tasks time to set themselves up
	nanosleep(10);

	long pid = fork();
	if (!pid)
		execve((const char *) "sh", (char * const *) "sh", /*NULL*/ 0);
	exit();
}
