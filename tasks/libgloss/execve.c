#include <errno.h>

#undef errno
extern int errno;

int execve(unsigned char *filepath, unsigned char *environ)
{
	long retvalue = sys_execve(filepath, environ);
	if (retvalue < 0)
	{
		errno = -retvalue;
		return -1;
	}
	else
		return retvalue;
}

