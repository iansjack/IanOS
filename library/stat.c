#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include "filesystem.h"

int stat(const char *filename, struct stat *st)
{
	int fd = open(filename, O_RDONLY);
	if (fd)
	{
		fstat(fd, st);
		errno = 0;
		return 0;
	}
	else
	{
		errno = -ENOENT;
		return 0;
	}
}

