#include "cmemory.h"
#include "ckstructs.h"

#define KBDINT 1

struct Message KbdMsg;
long kbBufStart;
long kbBufCurrent;
unsigned char kbBufCount;
unsigned char kbBuffer[128];

/*
====================================================
This is the task that listens for keyboard requests.
====================================================
*/
void kbTaskCode()
{
	CreatePTE(AllocPage64(), KernelStack);
	CreatePTE(AllocPage64(), UserStack);
	asm("mov $(0x3FF000 - 0x18), %rsp");
	kbBufStart = kbBufCurrent = kbBufCount = 0;
	
	((struct MessagePort *)ConsolePort)->waitingProc = 0;
	((struct MessagePort *)ConsolePort)->msgQueue = 0;
	while (1)
	{
		ReceiveMessage(KbdPort, &KbdMsg); 
		if (KbdMsg.byte == 1)
		{
			if (kbBufCount == 0) WaitForInt(KBDINT);
			unsigned char temp = kbBuffer[kbBufStart];
			kbBufCount--;
			kbBufStart++;
			if (kbBufStart == 128) kbBufStart = 0;
			struct MessagePort * tempPort = (struct MessagePort *)KbdMsg.quad;
			KbdMsg.quad = 0L;
			KbdMsg.byte = temp;
			SendMessage(tempPort, KbdMsg);
		}
		else
		{
		}
	}
}

/*
==============================
The lower-case keyboard table.
==============================
*/
char KbdTable[] = {'@', '@', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '@',
			'@', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,
		 	'@', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '#', '\'', '`', '@', '#',
		 	     'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
		 	'@', '@', '@', ' ', '@', '@', '@', '@', '@', '@', '@', '@', '@',
		 	'@', '@', '@', '@', '@', '@', '@', '@', '@', '@', '@', '@', '@',
		 	'@', '@', '@', '@', '@', '@', '@', '@', '@'};
