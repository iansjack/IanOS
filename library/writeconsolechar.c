#include "cmemory.h"
#include "ckstructs.h"
#include "library/syscalls.h"
#include "console.h"

void writeconsolechar(char c)
{
	struct Message * msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = WRITECHAR;
	msg->quad = c;
	sys_SendMessage(ConsolePort, msg);
	sys_DeallocMem(msg);
}
