#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
//	int test = open("TEST.TXT", O_RDWR);
//	printf("%d\n", test);
//	if (test != -1)
//	{
//		printf("Truncating file.\n");
//		sys_truncate(test, 10);
//		close(test);
//	}
	int test = open("newfile", O_RDWR | O_CREAT);
	printf("%d\n", test);
	if (test == -1)
		printf("%d\n", errno);
	else
		close(test);
	return (0);
}
