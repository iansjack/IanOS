#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	long t;
	printf("%d\n", sys_time());
	time(&t);
	printf("%d\n", t);
	int file = open("test.doc", O_RDWR);
	if (file)
	{
		printf("File opened\n");
		int retval = write(file, "ABC", 3);
		if (retval == -1) printf("Error writing to file\n");
		close(file);
	}
	return 0;
}
