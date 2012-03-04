#include "kernel.h"
#include "memory.h"
#include "filesystem.h"
#include "console.h"

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
copyMem(unsigned char *source, unsigned char *dest, long size)
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

//=======================================================
// Copy null-terminates string s1 to s2
//=======================================================
void copyString(unsigned char *source, unsigned char * destination)
{
	while (*source)
   	{
      *destination++ = *source++;
   	}
   	*destination = 0;
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
//  Returns a FD for the file in RAX
//=========================================================
FD KOpenFile(char *s)
{
   char *S = AllocKMem(12);
   char *str = S;
	struct FCB * fcb = 0;

   copyString(s, S);
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
   return -1;
}

//=========================================================
// 
//=========================================================
int KCloseFile(FD fileDescriptor)
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
		return 0;
	}
	return -1;
}

int DoStat(FD fileDescriptor, struct FileInfo *info)
{
	int retval = -1;
	
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
   		retval = 0; //msg->quad;
   		DeallocMem(msg);
	}
   	return (retval);
}

long DoRead(FD fileDescriptor, char *buffer, long noBytes)
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

long DoWrite(FD fileDescriptor, char *buffer, long noBytes)
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
		if (temp->deviceType == CONS)
		{
  			struct Message *FSMsg;

   			FSMsg = (struct Message *) AllocKMem(sizeof(struct Message));
   			char *buff = AllocKMem(noBytes + 1);
 			copyMem(buffer, buff, noBytes);
			Debug();
			buff[noBytes] = 0;

   			struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));
   			msg->nextMessage = 0;
   			msg->byte        = WRITESTR;
   			msg->quad        = (long) buff;
			msg->quad2       = 0; // sys_GetCurrentConsole();
   			SendMessage(ConsolePort, msg);
   			//DeallocMem(buff);
   			DeallocMem(msg);
			retval = noBytes;
		}
		else
		{
  			struct Message *FSMsg;

   			FSMsg = (struct Message *) AllocKMem(sizeof(struct Message));
   			char *buff = AllocKMem(noBytes);
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
	}
   return (retval);
}

FD DoCreate(char * s)
{
   	char *S = AllocKMem(12);
   	char *str = S;

   	copyString(s, S);
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
	return -1;
}