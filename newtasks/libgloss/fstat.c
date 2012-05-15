#include <sys/stat.h>
#include <errno.h>
#include "filesystem.h"

#undef errno
extern int errno;

int fstat(int fildes, struct stat *st)
{
	struct FileInfo inf;
	sys_fstat(fildes, &inf);
	st->st_mode = S_IFCHR;
  	st->st_size = inf.Length;
  	errno = 0;
  	return 0;
}

