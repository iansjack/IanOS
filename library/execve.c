#include <errno.h>

long sys_execve(const char *, char **);

int execve(const char *filepath, char **argv, char **env)
{
	long retvalue = sys_execve(filepath, argv);
	if (retvalue < 0)
	{
		errno = -retvalue;
		return -1;
	}
	else
		return retvalue;
}
