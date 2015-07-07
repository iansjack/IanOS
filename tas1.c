#include <unistd.h>

void nanosleep(long);	// Defined in scalls.s

// This task is just here to start the ball rolling

void tas1(void)
{
	long pid;
	// Give tasks time to set themselves up
//	nanosleep(1);

	pid = fork();
	if (!pid)
		execve((const char *) "netserver", (char * const *) "netserver", 0);
//	nanosleep(1);
	pid = fork();
	if (!pid)
		execve((const char *) "dhcp", (char * const *) "dhcp", /*NULL*/ 0);
	pid = fork();
//	nanosleep(1);
	if (!pid)
		execve((const char *) "sh", (char * const *) "sh", /*NULL*/ 0);

	exit(0);
}
