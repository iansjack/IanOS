#include "ckstructs.h"
#include "cmemory.h"
#include "library/syscalls.h"

char getchar(void);

void main(void)
{
	struct Message *taskMsg;
	int row = 0;
	int column = 0;
	char commandline[12];

	ClearScr();
	WriteString("Goodbye Cruel 32-bit World! Hello 64-bit.", 0, 0);
	row++;
	WriteString("#> ", row, 0);
	column = 3;
	WriteChar('_', row, column);
	sys_CreateTask("TASK2   BIN");
	while (1)
	{
		char c = getchar();
		switch (c)
		{
			//case 'p':
			//	taskMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
			//	taskMsg->nextMessage = 0;
			//	taskMsg->byte = 0x12;
			//	taskMsg->quad = 0x9876543212345678;
			//	Send(StaticPort, taskMsg);
			//	sys_DeallocMem(taskMsg);
			//	break;
			case 13:
				WriteString(commandline, 23, 0);
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
				WriteString(commandline, 24, 0);
				sys_CreateTask(commandline);
				break;
			default:
				WriteDouble(column, 24, 20);
				WriteDouble(row, 24, 40);
				commandline[column - 3] = c;
				WriteChar(c, row, column++);
				WriteChar('_', row, column);
				break;
		}
		WriteDouble(GetTicks(), 24, 60);
	}
}

char getchar(void)
{
	struct Message *kbdMsg;

	kbdMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	kbdMsg->nextMessage = 0;
	kbdMsg->byte = 1;
	kbdMsg->quad = 0;
	SendReceive(KbdPort, kbdMsg);
	char c = kbdMsg->byte;
	sys_DeallocMem(kbdMsg);
	return c;
}

