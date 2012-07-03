#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;

int close(int filedes)
{
	long retvalue = sys_close(filedes);
	if (retvalue < 0)
	{
		errno = -retvalue;
		return -1;
	}
	else
		return retvalue;
}

