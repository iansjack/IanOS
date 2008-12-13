#include "cmemory.h"
#include "ckstructs.h"

/*
====================================================
This is the task that listens for keyboard requests.
====================================================
*/
void consoleTaskCode()
{
	//struct MessagePort * CP = (struct MessagePort *)ConsolePort;
	((struct MessagePort *)ConsolePort)->waitingProc = 0;
	((struct MessagePort *)ConsolePort)->msgQueue = 0;
	
	short int column = 0;
	short int row = 0;
	struct Message ConsoleMsg;
	
	CreatePTE(AllocPage64(), KernelStack);
	CreatePTE(AllocPage64(), UserStack);
	asm("mov $(0x3FF000 - 0x18), %rsp");
	
	char * VideoBuffer = (char *)0xB8000;

	while (1)
	{
		ReceiveMessage(ConsolePort, &ConsoleMsg); 
		switch (ConsoleMsg.byte == 1)
		{
			case 1:
				switch (ConsoleMsg.quad)
				{
					case 9:
						column--;
						break;
					case 13:
						column = 0;
						row++;
						break;
					default:
						VideoBuffer[160 * row + 2 * column] = (unsigned char)ConsoleMsg.quad;
						column++;
						if (column == 80)
						{
							column = 0;
							row++;
						}
				}
				break;
			default:
				break;
		}
	}
}
