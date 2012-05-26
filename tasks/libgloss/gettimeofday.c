#include <errno.h>
#undef errno
extern int errno;

int gettimeofday(int mode)
{
	errno = EPERM;
	return -1;
}
