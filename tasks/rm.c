#include <stdio.h>
#include <stdlib.h>

extern errno;

int main(int argc, char **argv)
{
	if (unlink(argv[1]))
		printf("rm failed - errno = %d\n", errno);
	return (0);
}
