#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"
#include "console.h"

void
ConsoleClrScr()
{
   struct Message *msg =
         (struct Message *) sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte = CLRSCR;
   msg->quad = 0;
   msg->quad2 = sys_GetCurrentConsole();
   sys_SendMessage(ConsolePort, msg);
   sys_DeallocMem(msg);
}
