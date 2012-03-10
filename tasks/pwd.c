#include "syscalls.h"

int main(void)
{
	printf("%s\n", Sys_Getcwd());
	sys_Exit();
   	return(0);
}
