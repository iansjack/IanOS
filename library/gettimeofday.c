#include <errno.h>
#include <sys/time.h>

#undef errno
extern int errno;

int gettimeofday(struct timeval *tp, void * tzp)
{
	long time = sys_time();
	tp->tv_sec = time;
	tp->tv_usec = 0;
	errno = 0;
	return 0;
}
