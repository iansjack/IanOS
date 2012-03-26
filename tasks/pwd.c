#include "syscalls.h"

int main(void)
{
	unsigned char *cwd = getcwd();
	printf("%s\n", cwd);
	sys_DeallocMem(cwd);
	exit();
	return (0);
}
