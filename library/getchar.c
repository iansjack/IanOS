#include "syscalls.h"

char getchar()
{
	char b[2];
	Sys_Read(STDIN, b, 1);
	return b[0];
}
