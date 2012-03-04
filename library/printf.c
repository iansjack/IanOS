#include "syscalls.h"

void printf(char *s)
{
	int i = 0;
	while (s[i++]);
	Sys_Write(STDOUT, s, i - 1);
}
