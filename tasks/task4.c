#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int test = open("KERNLIB.C", O_RDWR);
	if (test != -1)
	{
		lseek(test, 1028, 0);
		close(test);
	}
	return (0);
}
