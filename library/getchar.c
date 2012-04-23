#include <syscalls.h>

char getchar()
{
	char b[2];
	read(STDIN, b, 1);
	return b[0];
}
