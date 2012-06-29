#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
	int file = open("/test", O_CREAT);
	printf("%d\n", file);
	if (file)
		close(file);
	return 0;
}
