//#include "kstructs.h"
//#include "syscalls.h"
//#include "lib.h"
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int ret;

	int InFile = open(argv[1]);
	if (InFile != -1) {
		struct stat inf;
		fstat(InFile, &inf);

		char *buffer = (char *)malloc(inf.st_size + 1);
		ret = read(InFile, buffer, inf.st_size);
		buffer[inf.st_size] = 0;
		printf("%s", buffer);
		close(InFile);
		free(buffer);
	}
	return (0);
}
