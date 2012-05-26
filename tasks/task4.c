#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int test = open("TEST.S", O_RDWR);
	printf("%d\n", test);
	if (test != -1)
	{
		char buffer[60];
		int ret = read(test, buffer, 20);
		printf("%d bytes read.\n", ret);
		printf("%s\n", buffer);
		close(test);
	}
	return (0);
}
