#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int test = open("TEST.TXT");
	printf("%d\n", test);
	if (test != -1)
	{
		printf("Truncating file.\n");
		sys_truncate(test, 10);
		close(test);
	}
	return (0);
}
