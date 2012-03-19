#include "syscalls.h"

int main(void)
{
	unsigned char *cwd = Sys_Getcwd();
	printf("%s\n", cwd);
	sys_DeallocMem(cwd);
	sys_Exit();
	return (0);
}
