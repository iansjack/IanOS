#include "syscalls.h"
#include "lib.h"
#include "console.h"

int main(void)
{
	char c[256];
	FD file = Sys_Open("TEST.TXT");
	int n = Sys_Read(file, c, 10);
	printf("%d\n", n);
	printf("%s\n", c);
	Sys_Close(file);
	char *ch = 0;
	printf("%c\n", ch[0]);
	sys_Exit();
}
