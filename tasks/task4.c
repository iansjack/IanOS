#include "syscalls.h"
#include "lib.h"

int main(void)
{
	unsigned char string[256]; // = "Hello World!!!"; 
	printf("%x\n", string);
	strcpy(string, "Hello World!!!");
	printf("%s\n", string);
	printf("%x\n", string);
	printf("%d\n", strlen(string));
	unsigned char *s1 = strcat(string, "W");
	printf("%x\n", s1);
	printf("%s\n", s1);
	printf("%d\n", strlen(string));
	sys_Exit();
   	return(0);
}

debug()
{
	while (1);
}