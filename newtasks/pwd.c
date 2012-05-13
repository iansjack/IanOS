#include <stdio.h>
#include "syscalls.h"

int main(int argc, char **argv)
{
	unsigned char *cwd;
	cwd = getcwd();
	printf("%s\n", cwd);
	return (0);
}
