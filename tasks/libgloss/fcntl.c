#include <errno.h>
#undef errno
extern int errno;

int fcntl(int filedes, int cmd)
{
	errno = EMFILE;
	return -1;
}
