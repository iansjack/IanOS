#include "../memory.h"
#include "../kstructs.h"
#include "lib.h"
#include "../filesystem.h"
#include "syscalls.h"
#include "../fat.h"

long
GetDirectoryEntry(int n, struct DirEntry * entry)
{
   struct Message *msg =
         (struct Message *) sys_AllocMem(sizeof(struct Message));
   char *buff = sys_AllocSharedMem(sizeof(struct DirEntry));
   int i;

   msg->nextMessage = 0;
   msg->byte = GETDIRENTRY;
   msg->quad = n;
   msg->quad2 = (long) buff;
   sys_SendReceive(FSPort, msg);
   for (i = 0; i < sizeof(struct DirEntry); i++)
   {
      ((char *) entry)[i] = ((char *) buff)[i];
   }
   sys_DeallocMem(buff);
   long retval = msg->quad;
   sys_DeallocMem(msg);
   return (retval);
}
