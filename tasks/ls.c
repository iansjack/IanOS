#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"
#include "fat.h"

int main(int argc, char **argv)
{
	struct DirEntry entry;
	int ret;
	int count1,count2;
	char buffer[13];

	FD file;
	if (argc == 2)
		file = Sys_Open(argv[1]);
	else 
		file = Sys_Open(Sys_Getcwd());
	if (file != -1)
	{
		for (count1 = 0; count1 < 40; count1++)
		{
			ret = Sys_Read(file, (char *)&entry, sizeof(struct DirEntry));
			if (entry.name[0]) // && entry.name[0] != 0xE5)
			{
				if (entry.name[0] != 0xE5)
				{
					for (count2 = 0; count2 < 11; count2++) 
			  			buffer[count2] = entry.name[count2];
		  			buffer[12] = 0;
		  			printf(buffer);
         			if (entry.attribute & 0x10)
            			printf(" <DIR> ");
         			else
            			printf("       ");
		  			buffer[8] = 0;
         			intToAsc(entry.fileSize, buffer, 8);
         			printf("%s\n", buffer);
				}
			}
			else
				break;
		}
		Sys_Close(file);
	}
	sys_Exit();
   	return(0);
}

