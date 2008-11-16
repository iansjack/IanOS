#include "cmemory.h"
#include "ckstructs.h"
#include "library/syscalls.h"

void main(void)
{
	WriteString("Task 4 is now running.", 14, 50);
	struct Message * msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = 1;
	msg->quad = 'Z';
	sys_SendMessage(ConsolePort, msg);
	while (1)
	{
		WriteDouble(GetTicks(), 23, 60);
	}
}
