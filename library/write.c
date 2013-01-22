#include <sys/stat.h>
#include <errno.h>

#undef errno
extern int errno;

int write(int fd, const void *buf, size_t count)
{
	long retvalue = sys_write(fd, buf, count);
	if (retvalue < 0)
	{
		errno = -retvalue;
		return -1;
	}
	else
		return retvalue;
}

