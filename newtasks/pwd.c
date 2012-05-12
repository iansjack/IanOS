#include "syscalls.h"

int main(int argc, char **argv)
{
	unsigned char *cwd;
	cwd = getcwd();
	printf("%s\n", cwd);
	free(cwd);
	exit();
	return (0);
}
