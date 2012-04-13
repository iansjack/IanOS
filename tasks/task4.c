#include "syscalls.h"
#include "lib.h"
#include "console.h"
#include "filesystem.h"

int main(int argc, char **argv)
{
	char * c = (char *)0x10000;

	FD InFile = open(argv[1]);
	if (InFile != -1) {
		struct FileInfo inf;
//		stat(InFile, &inf);

//		printf("%d\n", inf.Length);
//		char *buffer = malloc(inf.Length + 1);
//		ret = read(InFile, buffer, inf.Length);
//		buffer[inf.Length] = 0;
		write(InFile, c, 32);
		close(InFile);
//		free(buffer);
	}
	exit();
	return (0);
}
