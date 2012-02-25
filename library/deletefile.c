#include "memory.h"
#include "kstructs.h"
#include "lib.h"
#include "filesystem.h"
#include "syscalls.h"
#include "fat.h"

struct FCB *
DeleteFile(struct FCB *fHandle)
{
   struct Message *msg =
         (struct Message *) sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte = DELETEFILE;
   msg->quad = (long) fHandle;
   sys_SendReceive(FSPort, msg);
   return ((struct FCB *) msg->quad);
}
