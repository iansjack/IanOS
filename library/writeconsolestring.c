#include "memory.h"
#include "kstructs.h"
#include "syscalls.h"
#include "console.h"

void WriteConsoleString(char *s)
{
   char *S   = sys_AllocSharedMem(256);
   char *str = S;

   while (*s != 0)
   {
      *S++ = *s++;
   }
   *S = 0;
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   msg->nextMessage = 0;
   msg->byte        = WRITESTR;
   msg->quad        = (long)str;
	msg->quad2       = sys_GetCurrentConsole();
   sys_SendMessage(ConsolePort, msg);
   sys_DeallocMem(S);
   sys_DeallocMem(msg);
}
