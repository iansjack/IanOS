#include "kernel.h"
#include "memory.h"
#include "filesystem.h"

extern struct Task *currentTask;

//================================================
// Read noBytes into buffer from the file fHandle
//================================================
long
ReadFromFile(struct FCB *fHandle, char *buffer, long noBytes)
{
   long retval;

   struct Message *FSMsg;

   FSMsg = (struct Message *) AllocKMem(sizeof(struct Message));
   char *buff = AllocKMem(noBytes);
   int i;

   FSMsg->nextMessage = 0;
   FSMsg->byte = READFILE;
   FSMsg->quad = (long) fHandle;
   FSMsg->quad2 = (long) buff;
   FSMsg->quad3 = noBytes;
   SendReceiveMessage((struct MessagePort *) FSPort, FSMsg);
   for (i = 0; i < noBytes; i++)
   {
      buffer[i] = buff[i];
   }
   DeallocMem(buff);
   retval = FSMsg->quad;
   DeallocMem(FSMsg);
   return (retval);
}

//===========================================
// A utility function to copy a memory range
//===========================================
void
copyMem(unsigned char source[], unsigned char dest[], long size)
{
   int i;

   if (dest < (unsigned char *) 0x10000)
   {
      KWriteString("OOps!!!", 20, 40);
      asm("cli;"
            "hlt;");
   }

   for (i = 0; i < size; i++)
   {
      dest[i] = source[i];
   }
}

//===========================================================================
// A kernel library function to write a null-terminated string to the screen.
//===========================================================================
void
KWriteString(char *str, int row, int col)
{
   char *VideoBuffer = (char *) 0xB8000;

   int temp = 160 * row + 2 * col;
   int i = 0;
   while (str[i] != 0)
   {
      VideoBuffer[temp + 2 * i] = str[i];
      i++;
   }
}

//==========================================================================
// A kernel library function to write a quad character to the screen.
//==========================================================================
void
KWriteHex(long c, int row) //, int col)
{
   char *VideoBuffer = (char *) 0xB8000;

   int i;
   for (i == 0; i < 8; i++)
   {
      char lo = c & 0xF;
      lo += 0x30;
      if (lo > 0x39)
      {
         lo += 7;
      }
      char hi = (c & 0xF0) >> 4;
      hi += 0x30;
      if (hi > 0x39)
      {
         hi += 7;
      }
      VideoBuffer[160 * row + 28 - 4 * i] = hi;
      VideoBuffer[160 * row + 30 - 4 * i] = lo;
      c = c >> 8;
   }
}

/*long
strncmp(char * s1, char * s2, long length)
{
   long count;
   short done = 0;

   for (count = 1; count < length; count++)
   {
      if (s1[count] != s2[count])
      {
         done = 1;
         break;
      }
   }
   if (done)
      return (1);
   else
      return (0);
}
*/

//=========================================================
//  Opens the file s.
//  Returns a FCB for the file in RAX
//=========================================================
unsigned char KOpenFile(char *s)
{
   char *S = AllocKMem(12);
   char *str = S;
	struct FCB * fcb = 0;

   while (*s != 0)
   {
      *S++ = *s++;
   }
   *S = 0;
   struct Message *msg =
         (struct Message *) AllocKMem(sizeof(struct Message));
   msg->nextMessage = 0;
   msg->byte = OPENFILE;
   msg->quad = (long) str;
   SendReceiveMessage((struct MessagePort *)FSPort, msg);
	fcb = (struct FCB *) msg->quad;
   DeallocMem(S);
	if (fcb)
	{
		struct FCB * temp = currentTask->fcbList;
		int tempID = 2;
		while (temp->nextFCB && (temp->nextFCB->fileDescriptor == tempID))
		{
			temp = temp->nextFCB;
			tempID++;
		}
		fcb->nextFCB = temp->nextFCB;
		fcb->fileDescriptor = tempID;
		fcb->deviceType = FILE;
		temp->nextFCB = fcb;
		return tempID;
	}
   return 0;
}

//=========================================================
// 
//=========================================================
unsigned char KCloseFile(unsigned char fileDescriptor)
{
	struct FCB * temp = currentTask->fcbList;
	struct FCB * temp2;
	while (temp->nextFCB)
	{
		if (temp->nextFCB->fileDescriptor == fileDescriptor)
		{
			temp2 = temp->nextFCB;
			temp->nextFCB = temp->nextFCB->nextFCB;
			temp = temp2;
			break;
		}
		temp = temp->nextFCB;
	}
	if (temp)
	{
   		struct Message *msg =
    	     (struct Message *) AllocKMem(sizeof(struct Message));

   		msg->nextMessage = 0;
   		msg->byte = CLOSEFILE;
   		msg->quad = (long) temp /*fHandle*/;
   		SendReceiveMessage((struct MessagePort *)FSPort, msg);
//   		return ((struct FCB *) msg->quad);
		return 0;
	}
	return 1;
}

long DoStat(unsigned char fileDescriptor, struct FileInfo *info)
{
	long retval = 0;
	
	struct FCB * temp = currentTask->fcbList;
	while (temp->nextFCB)
	{
		if (temp->fileDescriptor == fileDescriptor)
			break;
		temp = temp->nextFCB;
	}
	if (temp)
	{
   		struct Message *msg =
        	 (struct Message *) AllocKMem(sizeof(struct Message));
   		char *buff = (char *) AllocKMem(sizeof(struct FileInfo));
 
   		msg->nextMessage = 0;
   		msg->byte = GETFILEINFO;
   		msg->quad = (long) temp;
   		msg->quad2 = (long) buff;
   		SendReceiveMessage((struct MessagePort *)FSPort, msg);
		copyMem(buff, (char *)info, sizeof(struct FileInfo));
   		DeallocMem(buff);
   		long retval = msg->quad;
   		DeallocMem(msg);
	}
   	return (retval);
}

long DoRead(unsigned char fileDescriptor, char *buffer, long noBytes)
{
	long retval = 0;
	
	struct FCB * temp = currentTask->fcbList;
	while (temp->nextFCB)
	{
		if (temp->fileDescriptor == fileDescriptor)
			break;
		temp = temp->nextFCB;
	}
	if (temp)
	{

   		struct Message *FSMsg;

   		FSMsg = (struct Message *)AllocKMem(sizeof(struct Message));
   		char *buff = AllocKMem(noBytes);
 
   		FSMsg->nextMessage = 0;
   		FSMsg->byte = READFILE;
   		FSMsg->quad = (long) temp;
   		FSMsg->quad2 = (long) buff;
   		FSMsg->quad3 = noBytes;
   		SendReceiveMessage((struct MessagePort *)FSPort, FSMsg);
		copyMem(buff, buffer, noBytes);
		DeallocMem(buff);
   		retval = FSMsg->quad;
   		DeallocMem(FSMsg);
	}
   return (retval);
}

long DoWrite(unsigned char fileDescriptor, char *buffer, long noBytes)
{
   long retval = 0;

 	struct FCB * temp = currentTask->fcbList;
	while (temp->nextFCB)
	{
		if (temp->fileDescriptor == fileDescriptor)
			break;
		temp = temp->nextFCB;
	}
	Debug();
	if (temp)
	{
  		struct Message *FSMsg;

   		FSMsg = (struct Message *) AllocKMem(sizeof(struct Message));
   		char *buff = AllocKMem(noBytes);
   		int i;
		copyMem(buffer, buff, noBytes);

   		FSMsg->nextMessage = 0;
   		FSMsg->byte = WRITEFILE;
   		FSMsg->quad = (long) temp;
   		FSMsg->quad2 = (long) buff;
   		FSMsg->quad3 = noBytes;
   		SendReceiveMessage((struct MessagePort *)FSPort, FSMsg);
   		DeallocMem(buff);
   		retval = FSMsg->quad;
   		DeallocMem(FSMsg);
	}
   return (retval);
}

unsigned char DoCreate(char * s)
{
   char *S = AllocKMem(12);
   char *str = S;

   while (*s != 0)
   {
      *S++ = *s++;
   }
   *S = 0;
   struct Message *msg =
         (struct Message *) AllocKMem(sizeof(struct Message));
   msg->nextMessage = 0;
   msg->byte = CREATEFILE;
   msg->quad = (long) str;
   SendReceiveMessage((struct MessagePort *)FSPort, msg);
   DeallocMem(S);
   long retval = msg->quad;
   DeallocMem(msg);
	struct FCB *fcb = (struct FCB *)retval;
	if (fcb)
	{
		struct FCB * temp = currentTask->fcbList;
		int tempID = 2;
		while (temp->nextFCB && (temp->nextFCB->fileDescriptor == tempID))
		{
			temp = temp->nextFCB;
			tempID++;
		}
		fcb->nextFCB = temp->nextFCB;
		fcb->fileDescriptor = tempID;
		fcb->deviceType = FILE;
		temp->nextFCB = fcb;
		return tempID;
	}
	return 0;
}