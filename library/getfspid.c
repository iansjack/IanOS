#include "memory.h"
#include "kstructs.h"
#include "lib.h"
#include "filesystem.h"
#include "syscalls.h"
#include "fat.h"

long
GetFSPID()
{
   struct Message *msg =
         (struct Message *) sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte = GETPID;
   sys_SendReceive(FSPort, msg);
   return (msg->quad);
}
