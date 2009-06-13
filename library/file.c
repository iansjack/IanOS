#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"
#include "filesystem.h"
#include "syscalls.h"
#include "fat.h"

struct FCB *CreateFile(char *s)
{
   char *S   = sys_AllocSharedMem(12);
   char *str = S;

   while (*s != 0)
   {
      *S++ = *s++;
   }
   *S = 0;
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   msg->nextMessage = 0;
   msg->byte        = CREATEFILE;
   msg->quad        = (long)str;
   sys_SendReceive(FSPort, msg);
	sys_DeallocMem(S);
	long retval = msg->quad;
	sys_DeallocMem(msg);
   return(retval);
}


struct FCB *OpenFile(char *s)
{
   char *S   = sys_AllocSharedMem(12);
   char *str = S;

   while (*s != 0)
   {
      *S++ = *s++;
   }
   *S = 0;
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   msg->nextMessage = 0;
   msg->byte        = OPENFILE;
   msg->quad        = (long)str;
   sys_SendReceive(FSPort, msg);
	sys_DeallocMem(S);
   return((struct FCB *)msg->quad);
}


struct FCB *CloseFile(struct FCB *fHandle)
{
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte        = CLOSEFILE;
   msg->quad        = (long)fHandle;
   sys_SendReceive(FSPort, msg);
   return((struct FCB *)msg->quad);
}


long ReadFile(struct FCB *fHandle, char *buffer, long noBytes)
{
   long retval;

   struct Message *FSMsg;

   FSMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   char *buff = sys_AllocSharedMem(noBytes);
   int  i;

   FSMsg->nextMessage = 0;
   FSMsg->byte        = READFILE;
   FSMsg->quad        = (long)fHandle;
   FSMsg->quad2       = (long)buff;
   FSMsg->quad3       = noBytes;
   sys_SendReceive(FSPort, FSMsg);
   for (i = 0; i < noBytes; i++)
   {
      buffer[i] = buff[i];
   }
   sys_DeallocMem(buff);
   retval = FSMsg->quad;
   sys_DeallocMem(FSMsg);
   return(retval);
}

long WriteFile(struct FCB *fHandle, char *buffer, long noBytes)
{
   long retval;

   struct Message *FSMsg;

   FSMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   char *buff = sys_AllocSharedMem(noBytes);
   int  i;
   for (i = 0; i < noBytes; i++)
   {
      buff[i] = buffer[i];
   }

   FSMsg->nextMessage = 0;
   FSMsg->byte        = WRITEFILE;
   FSMsg->quad        = (long)fHandle;
   FSMsg->quad2       = (long)buff;
   FSMsg->quad3       = noBytes;
   sys_SendReceive(FSPort, FSMsg);
   sys_DeallocMem(buff);
   retval = FSMsg->quad;
   sys_DeallocMem(FSMsg);
   return(retval);
}

struct FCB *DeleteFile(struct FCB *fHandle)
{
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte        = DELETEFILE;
   msg->quad        = (long)fHandle;
   sys_SendReceive(FSPort, msg);
   return((struct FCB *)msg->quad);
}

long GetFSPID()
{
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));

   msg->nextMessage = 0;
   msg->byte        = GETPID;
   sys_SendReceive(FSPort, msg);
   return(msg->quad);
}

long GetDirectoryEntry(int n, struct DirEntry * entry)
{
   struct Message *msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
   char *buff = sys_AllocSharedMem(sizeof(struct DirEntry));
	int i;
	
   msg->nextMessage = 0;
   msg->byte        = GETDIRENTRY;
	msg->quad			= n;
	msg->quad2			= (long) buff;
   sys_SendReceive(FSPort, msg);
   for (i = 0; i < sizeof(struct DirEntry); i++)
   {
      ((char *)entry)[i] = ((char *)buff)[i];
   }
   sys_DeallocMem(buff);
	long retval = msg->quad;
	sys_DeallocMem(msg);
   return(retval);
}