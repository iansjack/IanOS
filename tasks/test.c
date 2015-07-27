#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

int main(int argc, char **argv)
{
		long allocated = 0;
		char *buffer = malloc(1025 * 1025);;

		while (buffer)
		{
			allocated += 1025 * 1025;
			printf("Allocated: %ld\n", allocated);
			char *buffer = malloc(1025 * 1025);;
		}
	return (0);
}
