//#include "memory.h"
//#include "kstructs.h"
#include "syscalls.h"
//#include "console.h"

void printf(char *s)
{
	int i = 0;
	while (s[i++]);
	Sys_Write(STDOUT, s, i - 1);
}
