#include "memory.h"
#include "kstructs.h"
#include "lib.h"
#include "filesystem.h"
#include "syscalls.h"
#include "fat.h"

long
WriteFile(struct FCB *fHandle, char *buffer, long noBytes)
{
   long retval;

   struct Message *FSMsg;

   FSMsg = (struct Message *) sys_AllocMem(sizeof(struct Message));
   char *buff = sys_AllocSharedMem(noBytes);
   int i;
   for (i = 0; i < noBytes; i++)
   {
      buff[i] = buffer[i];
   }

   FSMsg->nextMessage = 0;
   FSMsg->byte = WRITEFILE;
   FSMsg->quad = (long) fHandle;
   FSMsg->quad2 = (long) buff;
   FSMsg->quad3 = noBytes;
   sys_SendReceive(FSPort, FSMsg);
   sys_DeallocMem(buff);
   retval = FSMsg->quad;
   sys_DeallocMem(FSMsg);
   return (retval);
}
