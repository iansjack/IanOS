#include "memory.h"
#include "kstructs.h"
//#include "lib.h"
//#include "library/syscalls.h"
#include "filesystem.h"
#include "syscalls.h"
#include "fat.h"

long
GetFileInfo(struct FCB *fHandle, struct FileInfo *info)
{
   struct Message *msg =
         (struct Message *) sys_AllocMem(sizeof(struct Message));
   char *buff = sys_AllocSharedMem(sizeof(struct FileInfo));
   int i;

   msg->nextMessage = 0;
   msg->byte = GETFILEINFO;
   msg->quad = (long) fHandle;
   msg->quad2 = (long) buff;
   sys_SendReceive(FSPort, msg);
   for (i = 0; i < sizeof(struct FileInfo); i++)
   {
      ((char *) info)[i] = ((char *) buff)[i];
   }
   sys_DeallocMem(buff);
   long retval = msg->quad;
   sys_DeallocMem(msg);
   return (retval);
}

