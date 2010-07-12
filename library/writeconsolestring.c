#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"
#include "console.h"

void writeconsolechar(char c, long console)
{
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte        = WRITECHAR;
   msg->quad        = c;
	msg->quad2       = console;
   sys_SendMessage(ConsolePort, msg);
   sys_DeallocMem(msg);
}


void writeconsolestring(char *s, long console)
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
	msg->quad2       = console;
   sys_SendMessage(ConsolePort, msg);
   sys_DeallocMem(S);
   sys_DeallocMem(msg);
}


void consoleclrscr(long console)
{
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte        = CLRSCR;
   msg->quad        = 0;
	msg->quad2       = console;
   sys_SendMessage(ConsolePort, msg);
   sys_DeallocMem(msg);
}
