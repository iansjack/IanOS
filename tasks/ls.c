#include "kstructs.h"
#include "library/syscalls.h"
#include "fat.h"

int main(void)
{
	struct DirEntry entry;
	int count;
	char buffer[8];

	for (count = 0; count < 200; count++)
	{
		GetDirectoryEntry(count, &entry);
		if (entry.name[0] != 0x0 && entry.name[0] != 0xE5)
		{
			writeconsolestring(entry.name);
			intToAsc(entry.fileSize, buffer, 8);
			writeconsolestring(buffer);
			writeconsolechar('\r');
		}
	}
	sys_KillTask();
   return(0);
}
