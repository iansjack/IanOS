#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;

int unlink(const char *filename)
{
	long retvalue = sys_unlink(filename);
	if (retvalue < 0)
	{
		errno = -retvalue;
		return -1;
	}
	else
		return retvalue;
}

