#include "cmemory.h"
#include "ckstructs.h"

struct Message ConsoleMsg;

/*
====================================================
This is the task that listens for keyboard requests.
====================================================
*/
void consoleTaskCode()
{
	CreatePTE(AllocPage64(), KernelStack);
	CreatePTE(AllocPage64(), UserStack);
	asm("mov $(0x3FF000 - 0x18), %rsp");
	
	char * VideoBuffer = (char *)0xB8000;
	
	while (1)
	{
		ReceiveMessage(ConsolePort, &ConsoleMsg); 
		if (ConsoleMsg.byte == 1)
		{
			VideoBuffer[0] = (unsigned char)ConsoleMsg.quad;
		}
		else
		{
		}
	}
}
