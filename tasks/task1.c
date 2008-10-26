#include "ckstructs.h"
#include "cmemory.h"
#include "library/syscalls.h"

void main(void)
{
	struct Message *taskMsg;
	struct Message *kbdMsg;

	//ClearScr();
	WriteString("Goodbye Cruel 32-bit World! Hello 64-bit.", 10, 0);
	sys_CreateTask("TASK2   BIN");
	while (1)
	{
		kbdMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
		kbdMsg->nextMessage = 0;
		kbdMsg->byte = 1;
		kbdMsg->quad = 0;
		SendReceive(KbdPort, kbdMsg);
		char c = kbdMsg->byte;
		sys_DeallocMem(kbdMsg);
		WriteChar(c, 11, 0);
		switch (c)
		{
			case 't':
				sys_CreateTask("TASK3   BIN");
				break;
			case 'p':
				taskMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
				taskMsg->nextMessage = 0;
				taskMsg->byte = 0x12;
				taskMsg->quad = 0x9876543212345678;
				Send(StaticPort, taskMsg);
				sys_DeallocMem(taskMsg);
				break;
			case 'k':
				sys_KillTask();
			default:
				break;
		}
		WriteDouble(GetTicks(), 20, 60);
	}
}
