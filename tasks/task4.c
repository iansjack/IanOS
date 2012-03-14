#include "syscalls.h"
#include "lib.h"
#include "console.h"

int main(void)
{
//	printf("First Line\n");
//	printf("%c[?5hHello%c[?5l World\n", ESC, ESC);
//	printf("Third Line\n");
//	Sys_Nanosleep(100);
//	printf("%cD", ESC);
//	Sys_Nanosleep(200);
//	printf("%cD", ESC);
	printf("%c[0;0H", ESC);
	printf("Moved%c[0K", ESC);
	printf("%c[1;1HMoved again\n", ESC);
	sys_Exit();
}