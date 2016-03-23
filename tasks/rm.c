#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv)
{
	if (unlink(argv[1]))
		printf("rm failed - errno = %d\n", errno);
	return (0);
}
