#include <errno.h>

long sys_execve(const char *, char **);

#undef errno
extern int errno;

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
