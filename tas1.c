#include <unistd.h>

// This task is just here to start the ball rolling

void tas1(void)
{
	long pid;

	pid = fork();
	if (!pid)
		execve((const char *) "netserver", (char * const *) "netserver", 0);
	pid = fork();
	if (!pid)
		execve((const char *) "dhcp", (char * const *) "dhcp", /*NULL*/ 0);
	pid = fork();
	if (!pid)
		execve((const char *) "sh", (char * const *) "sh", /*NULL*/ 0);

	exit(0);
}
