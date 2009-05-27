#include "../ckstructs.h"
#include "../cmemory.h"
#include "syscalls.h"

char getchar(void)
{
   struct Message *kbdMsg;

   kbdMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   kbdMsg->nextMessage = 0;
   kbdMsg->byte        = 1;
   kbdMsg->quad        = 0;
   SendReceive(KbdPort, kbdMsg);
   char c = kbdMsg->byte;
   sys_DeallocMem(kbdMsg);
   return(c);
}
