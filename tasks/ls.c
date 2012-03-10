#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"
#include "fat.h"

int main(void)
{
	struct DirEntry entry;
	int ret;
	int count1,count2;
	char buffer[13];
	
	FD descriptor = Sys_Open("/");
	if (descriptor != -1)
	{
		for (count1 = 0; count1 < 40; count1++)
		{
			ret = Sys_Read(descriptor, (char *)&entry, sizeof(struct DirEntry));
			if (entry.name[0] && entry.name[0] != 0xE5)
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
			else
				break;
		}
		Sys_Close(descriptor);
	}
	sys_Exit();
   	return(0);
}

