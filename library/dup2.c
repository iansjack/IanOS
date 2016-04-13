#include <sys/stat.h>
#include <errno.h>

long dup2(int oldfd, int newfd)
{
	long retvalue = sys_dup2(oldfd, newfd);
	if (retvalue < 0)
	{
		errno = -retvalue;
		return -1;
	}
	else
		return retvalue;
}

