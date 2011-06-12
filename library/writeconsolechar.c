#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"
#include "console.h"

void WriteConsoleChar(char c)
{
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte        = WRITECHAR;
   msg->quad        = c;
	msg->quad2       = sys_GetCurrentConsole();
   sys_SendMessage(ConsolePort, msg);
   sys_DeallocMem(msg);
}
