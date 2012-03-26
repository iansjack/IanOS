#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"

int main(int argc, char **argv)
{
	int ret;

	FD InFile = open(argv[1]);
	if (InFile != -1) {
		struct FileInfo inf;
		stat(InFile, &inf);

		char *buffer = (char *)sys_AllocMem(inf.Length + 1);
		ret = read(InFile, buffer, inf.Length);
		buffer[inf.Length] = 0;
		write(STDOUT, buffer, inf.Length + 1);
		close(InFile);
		sys_DeallocMem(buffer);
	}
	exit();
	return (0);
}
