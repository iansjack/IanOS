#include <sys/stat.h>
#include <errno.h>

long sys_open(char *, int);

#undef errno
extern int errno;

int open(char *filepath, int flags, ...)
{
	long retvalue = sys_open(filepath, flags);
	if (retvalue < 0)
	{
		errno = -retvalue;
		return -1;
	}
	else
		return retvalue;
}

