#include "../kstructs.h"
#include "../memory.h"
#include "syscalls.h"

char getchar()
{
   struct Message *kbdMsg;

   kbdMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   kbdMsg->nextMessage = 0;
   kbdMsg->byte        = 1;
   kbdMsg->quad        = sys_GetCurrentConsole();
   sys_SendReceive(KbdPort, kbdMsg);
   char c = kbdMsg->byte;
   sys_DeallocMem(kbdMsg);
   return(c);
}
