#include <errno.h>
//#include <sys/fcntl.h>
#undef errno
extern int errno;

int fcntl(int filedes, int cmd)
{
	if (cmd == 1 /*F_GETFD*/)
		return 0;
	errno = EMFILE;
	return -1;
}
