#include "ckstructs.h"
#include "cmemory.h"
#include "library/syscalls.h"
#include "library/lib.h"

void main(void)
{
	struct Message *taskMsg;
	int row = 0;
	int column = 0;
	char commandline[12];

	ClearScr();
	WriteString("IanOS Version 0.01 - 2008", 0, 0);
	row++;
	WriteString("#> ", row, 0);
	column = 3;
	WriteChar('_', row, column);
	sys_CreateTask("TASK2.BIN");
	while (1)
	{
		char c = getchar();
		switch (c)
		{
			case 13:
				column = 0;
				row++;
				WriteString("#> ", row, 0);
				column = 3;
				WriteChar('_', row, column);
				// Convert commandline[] to uppercase.
				char i;
				for (i = 0; i < 12; i++)
				{
					if (commandline[i] >= 'a' & commandline[i] <= 'z') 
						commandline[i] = commandline[i] - 0x20;
				}
				sys_CreateTask(commandline);
				break;
			default:
				commandline[column - 3] = c;
				WriteChar(c, row, column++);
				WriteChar('_', row, column);
				break;
		}
	}
}

