#include "syscalls.h"
#include "lib.h"

int main(void)
{
/*	unsigned char string[256]; // = "Hello World!!!"; 
	printf("%x\n", string);
	strcpy(string, "Hello World!!!");
	printf("%s\n", string);
	printf("%x\n", string);
	printf("%d\n", strlen(string));
	unsigned char *s1 = strcat(string, "W");
	printf("%x\n", s1);
	printf("%s\n", s1);
	printf("%d\n", strlen(string)); */
//	Sys_Fork();
	Sys_Debug(1);
	FD file = Sys_Open ("/TESTDIR/SUBDIR");
	Sys_Close(file);
	Sys_Debug(0);
	sys_Exit();
   	return(0);
}

debug()
{
	while (1);
}