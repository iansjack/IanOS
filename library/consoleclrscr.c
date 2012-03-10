#include "syscalls.h"
#include "console.h"

void
ConsoleClrScr()
{
	char s = CLRSCR;
	Sys_Write(STDOUT, &s, 1);
}
