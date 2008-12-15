#include "cmemory.h"
#include "ckstructs.h"
#include "console.h"

/*
====================================================
This is the task that listens for keyboard requests.
====================================================
*/

short int column;
short int row;
char * VideoBuffer;
struct Message ConsoleMsg;

void consoleTaskCode()
{
	CreatePTE(AllocPage64(), KernelStack);
	CreatePTE(AllocPage64(), UserStack);
	asm("mov $(0x3FF000 - 0x18), %rsp");

	column = 0;
	row = 0;
	VideoBuffer = (char *)0xB8000;
	((struct MessagePort *)ConsolePort)->waitingProc = 0;
	((struct MessagePort *)ConsolePort)->msgQueue = 0;

	unsigned char * s;
	
	while (1)
	{
		ReceiveMessage(ConsolePort, &ConsoleMsg); 
		switch (ConsoleMsg.byte)
		{
			case WRITECHAR:
				printchar((unsigned char)ConsoleMsg.quad);
				break;
			case WRITESTR:
				s = (unsigned char *)ConsoleMsg.quad;
				while (*s != 0)
				{
					printchar(*s);
					s++;
				} 
				DeallocMem(ConsoleMsg.quad);
				break;
			default:
				break;
		}
	}
}

void printchar(unsigned char c)
{
	switch (c)
	{
		case 9:
			if (column > 0) column--;
			break;
		case 13:
			column = 0;
			row++;
			break;
		default:
			VideoBuffer[160 * row + 2 * column] = c;
			column++;
			if (column == 80)
			{
				column = 0;
				row++;
			}
	}
}
