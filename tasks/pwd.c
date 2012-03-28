#include "syscalls.h"

int main(void)
{
	unsigned char *cwd = getcwd();
	printf("%s\n", cwd);
	free(cwd);
	exit();
	return (0);
}
