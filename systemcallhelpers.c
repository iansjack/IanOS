#include <console.h>
#include <errno.h>
#include <linux/types.h>
#include <time.h>
#include <fcntl.h>
#include <kernel.h>
#include <filesystem.h>
#include "blocks.h"

typedef long unsigned int size_t;

extern struct Task *currentTask;
extern struct MessagePort *FSPort;
extern struct MessagePort *KbdPort;
extern struct MessagePort *ConsolePort;

//==================================================
// Find the first free file descriptor for the task
//==================================================
long FindFirstFreeFD()
{
	unsigned char n;
	for (n = 0; n < 16; n++)
		if (!(currentTask->fcb[n]))
			return n;
	return -EMFILE;
}

long BadFile(struct FCB *fHandle, char *buffer, long nBytes)
{
	return (-EBADF);
}

//============================================================
// Read nBytes from keyboard. Will always read just one byte.
//============================================================
long ReadFromKeyboard(struct FCB *fHandle, char *buffer, long nBytes)
{
	struct Message *kbdMsg;
	char c;

	kbdMsg = ALLOCMSG;
	kbdMsg->nextMessage = 0;
	kbdMsg->byte = 1; // GETCHAR message
	kbdMsg->quad1 = currentTask->console;
	SendReceiveMessage(KbdPort, kbdMsg);
	c = (char) (kbdMsg->byte);
	buffer[0] = c;
	buffer[1] = 0;
	DeallocMem(kbdMsg);
	return (1);
}

//================================================
// Read noBytes into buffer from the file fHandle
//================================================
long ReadFromFile(struct FCB *fHandle, char *buffer, long noBytes)
{
	long retval = 0;
	long bytesRead = 0;
	char done = 0;
	char *buff;
	struct Message *FSMsg;

	if (noBytes == 0)
		return 0;

	buff = AllocKMem(PageSize);
	FSMsg = ALLOCMSG;

	while (noBytes > PageSize)
	{
		FSMsg->nextMessage = 0;
		FSMsg->byte = READFILE;
		FSMsg->quad1 = (long) fHandle;
		FSMsg->quad2 = (long) buff;
		FSMsg->quad3 = PageSize;
		SendReceiveMessage(FSPort, FSMsg);
		memcpy(buffer + bytesRead, buff, (size_t) (FSMsg->quad1));
		bytesRead += FSMsg->quad1;
		noBytes -= PageSize;
		if (FSMsg->quad1 < PageSize)
			done = 1;
	}
	if (!done)
	{
		FSMsg->nextMessage = 0;
		FSMsg->byte = READFILE;
		FSMsg->quad1 = (long) fHandle;
		FSMsg->quad2 = (long) buff;
		FSMsg->quad3 = noBytes;
		SendReceiveMessage(FSPort, FSMsg);
		memcpy(buffer + bytesRead, buff, (size_t) (FSMsg->quad1));
		bytesRead += FSMsg->quad1;
	}

	retval = bytesRead;
	DeallocMem(buff);
	DeallocMem(FSMsg);
	return (retval);
}

//==============================
// Write noBytes to the Console
//==============================
long WriteToConsole(struct FCB *fHandle, char *buffer, long noBytes)
{
	char *buff = AllocKMem((size_t) noBytes + 1);
	struct Message *msg = ALLOCMSG;

	memcpy(buff, buffer, (size_t) noBytes);
	buff[noBytes] = 0;
	msg->nextMessage = 0;
	msg->byte = WRITESTR;
	msg->quad1 = (long) buff;
	msg->quad2 = currentTask->console;
	SendMessage(ConsolePort, msg);
	DeallocMem(msg);
	return noBytes;
}

//==================================================
// Write noBytes to the file represented by fHandle
//==================================================
long WriteToFile(struct FCB *fHandle, char *buffer, long noBytes)
{
	struct Message *FSMsg;
	char *buff;
	long retval;

	FSMsg = ALLOCMSG;
	buff = AllocKMem((size_t) noBytes);
	memcpy(buff, buffer, (size_t) noBytes);

	FSMsg->nextMessage = 0;
	FSMsg->byte = WRITEFILE;
	FSMsg->quad1 = (long) fHandle;
	FSMsg->quad2 = (long) buff;
	FSMsg->quad3 = noBytes;
	SendReceiveMessage(FSPort, FSMsg);
	DeallocMem(buff);
	retval = FSMsg->quad1;
	DeallocMem(FSMsg);
	return retval;
}

void CloseAFile(struct FCB *file)
{
	struct Message *msg;

	msg = ALLOCMSG;

	msg->nextMessage = 0;
	msg->byte = CLOSEFILE;
	msg->quad1 = (long) file;
	SendReceiveMessage(FSPort, msg);
	DeallocMem(msg);
}

void DummyClose(struct FCB *file)
{
}

//=========================================================
//  Opens the file s.
//  Returns a FD for the file
//=========================================================
FD DoOpen(char *s, int flags)
{
	struct FCB *fcb = 0;

	char *S = NameToFullPath(s);
	struct Message *msg = ALLOCMSG;
	msg->nextMessage = 0;
	msg->byte = OPENFILE;
	msg->quad1 = (long) S;
	SendReceiveMessage(FSPort, msg);
	fcb = (struct FCB *) msg->quad1;
	if ((long) fcb <= 0)
	{
		if (flags & 0x200 /*O_CREAT*/) // Why is O_CREAT ending up as 0x40???
		{
			msg->nextMessage = 0;
			msg->byte = CREATEFILE;
			msg->quad1 = (long) S;
			SendReceiveMessage(FSPort, msg);
			fcb = (struct FCB *) msg->quad1;
		}
	}
	DeallocMem(S);
	DeallocMem(msg);

	if ((long) fcb > 0)
	{
		struct FCB *temp;
		FD fileDescriptor = FindFirstFreeFD();
		if (fileDescriptor == -EMFILE)
			return -EMFILE;
		currentTask->fcb[fileDescriptor] = fcb;
		if (flags & 0x3 == O_RDONLY)
			fcb->write = BadFile;
		else
			fcb->write = WriteToFile;
		if (flags & 0x3 == O_WRONLY)
			fcb->read = BadFile;
		else
			fcb->read = ReadFromFile;
		fcb->close = CloseAFile;
		return fileDescriptor;
	}
	else
		return -EBADF;
}

//=========================================================
//  Closes a file
//=========================================================
int DoClose(FD fileDescriptor)
{
	struct FCB *temp;

	temp = currentTask->fcb[fileDescriptor];
	if (temp)
	{
		temp->close;
		return 0;
	}
	return -EBADF;
}

//=================================
// Implements the stat system call
//=================================
int DoStat(char *path, struct FileInfo *info)
{
	char *S = NameToFullPath(path);
	struct Message *msg = ALLOCMSG;
	char *buff = (char *) AllocKMem(sizeof(struct FileInfo));
	struct FCB *fcb;

	msg->nextMessage = 0;
	msg->byte = OPENFILE;
	msg->quad1 = (long) S;
	SendReceiveMessage(FSPort, msg);
	fcb = (struct FCB *) msg->quad1;
	if ((long) fcb > 0)
	{
		msg->nextMessage = 0;
		msg->byte = GETFILEINFO;
		msg->quad1 = (long) fcb;
		msg->quad2 = (long) buff;
		SendReceiveMessage(FSPort, msg);
		memcpy((char *) info, buff, sizeof(struct FileInfo));
		DeallocMem(buff);
		msg->nextMessage = 0;
		msg->byte = CLOSEFILE;
		msg->quad1 = (long) fcb;
		SendReceiveMessage(FSPort, msg);
		DeallocMem(msg);
		return 0;
	}
	return (long) fcb;
}

//=================================
//Implements the fstat system call
//=================================
int DoFStat(FD fileDescriptor, struct FileInfo *info)
{
	struct FCB *temp = currentTask->fcb[fileDescriptor];
	if (temp)
	{
		if (temp->deviceType == KBD || temp->deviceType == CONS)
		{
			info->size = 0;
		}
		else
		{
			struct Message *msg = ALLOCMSG;
			char *buff = (char *) AllocKMem(sizeof(struct FileInfo));

			msg->nextMessage = 0;
			msg->byte = GETFILEINFO;
			msg->quad1 = (long) temp;
			msg->quad2 = (long) buff;
			SendReceiveMessage(FSPort, msg);
			memcpy((char *) info, buff, sizeof(struct FileInfo));
			DeallocMem(buff);
			DeallocMem(msg);
			return 0;
		}
	}
	return -EBADF;
}

//=================================
// Implements the read system call
//=================================
long DoRead(FD fileDescriptor, char *buffer, long noBytes)
{
	struct FCB *temp = currentTask->fcb[fileDescriptor];

	if (!noBytes)
		return 0;

	if (temp)
		return(temp->read(temp, buffer, noBytes));
	return (0);
}

//==================================
// Implements the write system call
//==================================
long DoWrite(FD fileDescriptor, char *buffer, long noBytes)
{
	if (!noBytes)
		return 0;

	struct FCB *temp = currentTask->fcb[fileDescriptor];
	if (temp)
		return (temp->write(temp, buffer, noBytes));
	return (0);
}

//==================================
// Implements the creat system call
//==================================
FD DoCreate(char *s)
{
	struct FCB *fcb;
	long retval = 0;
	char *S = NameToFullPath(s);
	struct Message *msg = ALLOCMSG;
	msg->nextMessage = 0;
	msg->byte = CREATEFILE;
	msg->quad1 = (long) S; //str;
	SendReceiveMessage(FSPort, msg);
	DeallocMem(S);
	retval = msg->quad1;
	DeallocMem(msg);
	fcb = (struct FCB *) retval;
	if ((long) fcb > 0)
	{
		fcb->write = WriteToFile;
		fcb->read = ReadFromFile;

		FD fileDescriptor = FindFirstFreeFD();
		if (fileDescriptor == -EMFILE)
			return -EMFILE;
		currentTask->fcb[fileDescriptor] = fcb;
		return fileDescriptor;
	}
	return -ENFILE;
}

//==================================
// Implements the mkdir system call
//==================================
long DoMkDir(char *s)
{
	long retval = 0;
	char *S = NameToFullPath(s);
	struct Message *msg = ALLOCMSG;
	msg->nextMessage = 0;
	msg->byte = CREATEDIR;
	msg->quad1 = (long) S;
	SendReceiveMessage(FSPort, msg);
	DeallocMem(S);
	retval = msg->quad1;
	DeallocMem(msg);
	return retval;
}

//===================================
// Implements the delete system call
//===================================
long DoDelete(char *name)
{
	int retval;
	char *S = NameToFullPath(name);
	struct Message *msg = ALLOCMSG;

	msg->nextMessage = 0;
	msg->byte = DELETEFILE;
	msg->quad1 = (long) S;
	SendReceiveMessage(FSPort, msg);
	retval = msg->quad1;
	DeallocMem(S);
	DeallocMem(msg);
	return retval;
}

//===================================
// Implements the chdir system call
//===================================
long DoChDir(char *dirName)
{
	char *S = NameToFullPath(dirName);
	struct Message *msg = ALLOCMSG;
	long retval = 0;

	msg->nextMessage = 0;
	msg->byte = TESTFILE;
	msg->quad1 = (long) S;
	SendReceiveMessage(FSPort, msg);
	retval = msg->quad1;
	if (retval > 0)
	{
		DeallocMem(currentTask->currentDirName);
		currentTask->currentDirName = AllocKMem((size_t) strlen(S) + 1);
		strcpy(currentTask->currentDirName, S);
	}
	DeallocMem(S);
	DeallocMem(msg);
	return retval;
}

//===================================
// Implements the getcwd system call
//===================================
char *DoGetcwd(char *name, long length)
{
	if (length < strlen(currentTask->currentDirName))
		return (char *) -ERANGE;
	strcpy(name, currentTask->currentDirName);
	return name;
}

//====================================
// A wrapper for the seek system call
//====================================
long SeekFile(struct Message *FSMsg, struct FCB * fHandle, long offset,
		long whence)
{
	long retval = 0;

	FSMsg->nextMessage = 0;
	FSMsg->byte = SEEK;
	FSMsg->quad1 = (long) fHandle;
	FSMsg->quad2 = (long) offset;
	FSMsg->quad3 = (long) whence;
	SendReceiveMessage(FSPort, FSMsg);
	retval = FSMsg->quad1;

	return retval;
}

//===================================
// Implements the seek system call
//===================================
int DoSeek(FD fileDescriptor, int offset, int whence)
{
	long retval = 0;
	struct FCB *temp = currentTask->fcb[fileDescriptor];
	if (temp)
	{
		if (temp->deviceType == CONS)
		{
			return -EINVAL;
		}
		else
		{
			struct Message *FSMsg = ALLOCMSG;

			retval = SeekFile(FSMsg, temp, offset, whence);
			DeallocMem(FSMsg);
			return retval;
		}
	}
	return -EBADF;
}

//=====================================
// Implements the truncate system call
//=====================================
int DoTruncate(FD fileDescriptor, long length)
{
	long retval = 0;
	struct FCB *temp = currentTask->fcb[fileDescriptor];
	if (temp)
	{
		if (temp->deviceType == CONS)
		{
			return -EINVAL;
		}
		else
		{
			struct Message *FSMsg = ALLOCMSG;

			FSMsg->nextMessage = 0;
			FSMsg->byte = TRUNCATE;
			FSMsg->quad1 = (long) temp;
			FSMsg->quad2 = (long) length;
			SendReceiveMessage(FSPort, FSMsg);
			retval = FSMsg->quad1;
			DeallocMem(FSMsg);
			return retval;
		}
	}
	return -EBADF;
}

void GoToSleep(long timeout)
{
	struct MessagePort *port = AllocMessagePort();
	struct Message msg;

	newtimer(timeout, port);
	ReceiveMessage(port, &msg);
}

DoDup2(int oldfd, int newfd)
{
	if (currentTask->fcb[newfd])
	{
		DoClose(newfd);
	}
	else
	{
		currentTask->fcb[newfd] = currentTask->fcb[oldfd];
		currentTask->fcb[newfd]->openCount++;
	}
}
