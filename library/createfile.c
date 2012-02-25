#include "memory.h"
#include "kstructs.h"
#include "lib.h"
//#include "library/syscalls.h"
#include "filesystem.h"
#include "syscalls.h"
#include "fat.h"

struct FCB *
CreateFile(char *s)
{
   char *S = sys_AllocSharedMem(12);
   char *str = S;

   while (*s != 0)
   {
      *S++ = *s++;
   }
   *S = 0;
   struct Message *msg =
         (struct Message *) sys_AllocMem(sizeof(struct Message));
   msg->nextMessage = 0;
   msg->byte = CREATEFILE;
   msg->quad = (long) str;
   sys_SendReceive(FSPort, msg);
   sys_DeallocMem(S);
   long retval = msg->quad;
   sys_DeallocMem(msg);
   return ((struct FCB *) retval);
}
