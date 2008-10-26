#include "cmemory.h"
#include "ckstructs.h"
#include "library/syscalls.h"

void main(void)
{
	WriteString("Task 3 is now running.", 13, 0);
	struct Message * msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = 0;
	msg->quad = 0;
	sys_ReceiveMessage(StaticPort, msg);
	WriteString("Message was received from Task 1.", 14, 0);
	WriteDouble(msg->quad, 14, 60);
	while (1);
}
