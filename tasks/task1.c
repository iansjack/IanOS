#include "ckstructs.h"
#include "cmemory.h"
#include "library/syscalls.h"
#include "library/lib.h"

void main(void)
{
	struct Message *taskMsg;
	//int row = 0;
	int column = 0;
	char commandline[80];

	ClearScr();
	WriteString("IanOS Version 0.01 - 2008", 0, 0);
	writeconsolechar(13);
	writeconsolechar('#');
	writeconsolechar('>');
	writeconsolechar(' ');
	writeconsolechar('_');
	writeconsolechar(9);
	sys_CreateTask("TASK2.BIN");
	while (1)
	{
		char c = getchar();
		switch (c)
		{
			case 13:
				column = 0;
				writeconsolechar(' ');
				writeconsolechar(13);
				writeconsolechar('#');
				writeconsolechar('>');
				writeconsolechar(' ');
				writeconsolechar('_');
				writeconsolechar(9);
				// Convert commandline[] to uppercase.
				char i;
				for (i = 0; i < 12; i++)
				{
					if (commandline[i] >= 'a' & commandline[i] <= 'z') 
						commandline[i] = commandline[i] - 0x20;
				}
				sys_CreateTask(commandline);
				// Clear commandline[]
				for (i = 0; i < 12; i++) commandline[i] = 0;
				break;
			default:
				commandline[column++] = c;
				writeconsolechar(c);
				writeconsolechar('_');
				writeconsolechar(9);
				break;
		}
	}
}

