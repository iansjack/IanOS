#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"

int main(int argc, char **argv)
{
	int ret;

	FD InFile = Sys_Open(argv[1]);
	if (InFile != -1) {
		struct FileInfo inf;
		Sys_Stat(InFile, &inf);

		char *buffer = (char *)sys_AllocMem(inf.Length + 1);
		ret = Sys_Read(InFile, buffer, inf.Length);
		buffer[inf.Length] = 0;
		Sys_Write(STDOUT, buffer, inf.Length + 1);
		Sys_Close(InFile);
		sys_DeallocMem(buffer);
	}
	sys_Exit();
	return (0);
}
