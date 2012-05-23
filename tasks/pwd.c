#include <stdio.h>
#include "syscalls.h"

int main(int argc, char **argv)
{
	unsigned char *cwd = malloc(16);
	cwd = getcwd(cwd, 16);
	printf("%s\n", cwd);
	free(cwd);
	return (0);
}
