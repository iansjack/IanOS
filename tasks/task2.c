#include "kstructs.h"
#include "library/syscalls.h"
#include "library/lib.h"
#include "fat.h"

int main(void)
{
	struct DirEntry entry;

	sys_WriteString("Hi", 22, 40);
	long pid = GetFSPID();
	sys_WriteDouble(pid, 23, 20);
	struct FCB *fHandle = CreateFile("TEST.TXT");
	sys_WriteDouble((long) fHandle, 23, 40);
	if (fHandle)
	{
		WriteFile(fHandle, "1234\n", 5);
		Sys_Close(fHandle);
	}
	sys_Sleep(200);
	int count;
	for (count = 0; count < 5; count++)
	{
		GetDirectoryEntry(count, &entry);
		sys_WriteString((unsigned char *) &entry, 23, 0);
		sys_Sleep(100);
	}
	sys_Exit();
	return (0);
}
