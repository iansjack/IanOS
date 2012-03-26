#include "syscalls.h"
#include "lib.h"
#include "console.h"
#include "filesystem.h"

int main(void)
{
	char c[256];
	FD file = open("TEST.TXT");
	int n = read(file, c, 1);
	printf("%c\n", c[0]);
	lseek(file, 3, SEEK_SET);
	read(file, c, 1);
	printf("%c\n", c[0]);
	lseek(file, -1, SEEK_CUR);
	read(file, c, 1);
	printf("%c\n", c[0]);
	lseek(file, -2, SEEK_END);
	read(file, c, 1);
	printf("%c\n", c[0]);
	close(file);
//	Sys_MkDir("NEWDIR");
	exit();
}
