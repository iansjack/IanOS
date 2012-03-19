#include "syscalls.h"
#include "lib.h"
#include "console.h"

int main(void)
{
//      printf("First Line\n");
//      printf("%c[?5hHello%c[?5l World\n", ESC, ESC);
//      printf("Third Line\n");
//      Sys_Nanosleep(100);
//      printf("%cD", ESC);
//      Sys_Nanosleep(200);
//      printf("%cD", ESC);
	printf("%c[0;0H", ESC);
	printf("Moved%c[0K", ESC);
	printf("%c[1;1HMoved again", ESC);
	printf("%c[3D", ESC);
	printf("Left");
	printf("%c[10;0H", ESC);
	char *c = 0x10000;
	Sys_Nanosleep(1000);
	// This should produce a Page Fault, showing that address 0 cannot be read. It does!!!
	printf("%c", c[0]);
	sys_Exit();
}
