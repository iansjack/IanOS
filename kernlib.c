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

/* extern long sec, min, hour, day, month, year, unixtime;

void PrintClock()
{
	kprintf(0, 63, "                 ");
//	kprintf(0, 63, "%2d/%02d/%02d %2d:%02d:%02d", day, month, year, hour, min,
//			sec);
	kprintf(0, 63, "%d", unixtime);
}

void setclock()
{
	struct tm tm;
	tm.tm_sec = sec;
	tm.tm_min = min;
	tm.tm_hour = hour;
	tm.tm_mday = day;
	tm.tm_mon = month - 1;
	tm.tm_year = year + 100;
	tm.tm_isdst = -1;
	//tm.tm_gmtoff = 0;
	unixtime = (long) mktime(&tm);
}
*/

long FindFirstFreeFD()
{
	unsigned char n;
	for (n = 0; n < 16; n++)
		if (!(currentTask->fcb[n]))
			return n;
	return -EMFILE;
}

/*//===============================================================
// Convert a file descriptor for the current process to an FCB
//===============================================================
struct FCB *fdToFCB(FD fileDescriptor)
{
	struct FCB *temp = currentTask->fcbList;
	while (temp)
	{
		if (temp->fileDescriptor == fileDescriptor)
			break;
		temp = temp->nextFCB;
	}
	return temp;
}
*/
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

//===========================================================================
// A kernel library function to write a null-terminated string to the screen.
//===========================================================================
void KWriteString(char *str, int row, int col)
{
	char *VideoBuffer = (char *) 0x80000B8000;

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
void KWriteHex(long c, int row) //, int col)
{
	char *VideoBuffer = (char *) 0x80000B8000;
	int i;
	char hi;

	for (i = 0; i < 8; i++)
	{
		char lo = (char)(c & 0xF);
		lo += 0x30;
		if (lo > 0x39)
		{
			lo += 7;
		}
		hi = (char)((c & 0xF0) >> 4);
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
	if ((long)fcb <= 0)
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
		fcb->mode = flags;
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
	struct FCB *temp, *temp2;

	temp = currentTask->fcb[fileDescriptor];
	if (temp)
	{
		struct Message *msg;

		if (temp->deviceType == KBD || temp->deviceType == CONS)
			return 0;
		msg = ALLOCMSG;

		msg->nextMessage = 0;
		msg->byte = CLOSEFILE;
		msg->quad1 = (long) temp;
		SendReceiveMessage(FSPort, msg);
		DeallocMem(msg);
		return 0;
		DeallocMem(currentTask->fcb[fileDescriptor]);
		currentTask->fcb[fileDescriptor] = 0;
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
	fcb = (struct FCB *)msg->quad1;
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
	return (long)fcb;
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
	long retval = 0;
	struct FCB *temp;

	if (!noBytes) return 0;

	temp = currentTask->fcb[fileDescriptor];
	if (temp)
	{
		if (temp->deviceType == KBD)
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
		else
		{
			if (temp->deviceType == CONS) return (-EINVAL);
			if ((temp->mode & 3) == O_WRONLY)return (-EBADF);
			retval = ReadFromFile(temp, buffer, noBytes);
		}
	}
	return (retval);
}

//==================================
// Implements the write system call
//==================================
long DoWrite(FD fileDescriptor, char *buffer, long noBytes)
{
	long retval = 0;
	struct FCB *temp;

	if (!noBytes) return 0;

	temp = currentTask->fcb[fileDescriptor];
	if (temp)
	{
		if (temp->deviceType == CONS)
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
			retval = noBytes;
		}
		else
		{
			struct Message *FSMsg;
			char *buff;

			if (temp->deviceType == KBD) return (-EINVAL);
			if ((temp->mode & 3) == O_RDONLY)return (-EBADF);

			FSMsg = ALLOCMSG;
			buff = AllocKMem((size_t) noBytes);
			memcpy(buff, buffer, (size_t) noBytes);

			FSMsg->nextMessage = 0;
			FSMsg->byte = WRITEFILE;
			FSMsg->quad1 = (long) temp;
			FSMsg->quad2 = (long) buff;
			FSMsg->quad3 = noBytes;
			SendReceiveMessage(FSPort, FSMsg);
			DeallocMem(buff);
			retval = FSMsg->quad1;
			DeallocMem(FSMsg);
		}
	}
	return (retval);
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
		struct FCB *temp;
		FD fileDescriptor = FindFirstFreeFD();
		if (fileDescriptor == -EMFILE)
			return -EMFILE;
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
	long retval =0;

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
long SeekFile(struct Message *FSMsg, struct FCB * fHandle, long offset, long whence)
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

char *NameToFullPath(char *name)
{
	char *S = AllocKMem((size_t) (strlen(name) + strlen(currentTask->currentDirName) + 3));

	// "." is a very special case. It changes nothing.
	if (!strcmp(name, "."))
	{
		strcpy(S, currentTask->currentDirName);
		return S;
	}
	// The special case ".."
	if (!strcmp(name, ".."))
	{
		if (!strcmp(currentTask->currentDirName, "/"))
		{
			strcpy(S, "/");
			return S;
		}
		else
		{
			strcpy(S, currentTask->currentDirName);
			while (S[strlen(S) - 1] != '/')
				S[strlen(S) - 1] = 0;
			S[strlen(S) - 1] = 0;
			if (strlen(S) == 0)
				S[0] = '/';
		}
	}
	else
	{
		// The special case "./file"
		if (name[0] == '.' && name[1] == '/')
			name += 2;
		// The special case "../file"
		if (name[0] == '.' && name[1] == '.' && name[2] == '/')
		{
			name += 3;
			strcpy(S, currentTask->currentDirName);
			while (S[strlen(S) - 1] != '/')
				S[strlen(S) - 1] = 0;
			strcat(S, name);
		}
		else
		{
			strcpy(S, "");
			if (!strcmp(name, "/"))
				strcpy(S, "/");
			else
			{
				if (name[0] == '/')
					strcpy(S, name);
				else
				{
					if (strcmp(currentTask->currentDirName, "/"))
						strcpy(S, currentTask->currentDirName);
					strcat(S, "/");
					strcat(S, name);
				}
			}
		}
	}
	return S;
}

#include <stdarg.h>

int kprintf(int row, int column, char *s, ...)
{
	char sprocessed[256];
	int i = 0;
	int j = 0;
	int k = 0;
	short minwidth = 0;
	short zeropadding = 0;
	short indicator = 0;

	va_list ap;
	va_start(ap, s);

	for (i = 0; i < 256; i++)
		sprocessed[i] = 0;

	i = 0;

	while (s[i])
	{
		minwidth = 0;
		zeropadding = 0;
		indicator = 0;
		k = 0;

		if (s[i] != '%')
		{
			sprocessed[j] = s[i];
			i++;
			j++;
		}
		else
		{
			i++;
			if (s[i] == '#')
			{
				indicator = 1;
				i++;
			}
			if (s[i] == '0')
			{
				zeropadding = 1;
				i++;
			}
			if ('0' < s[i] && s[i] <= '9') // A width specifier
			{
				minwidth = s[i] - '0';
				i++;
			}
			switch (s[i])
			{
			int c, number;
			char *s1;
			char buffer[20];

			case 'c':
				while (minwidth-- > 1)
					sprocessed[j++] = ' ';
				c = va_arg(ap, int);
				sprocessed[j++] = c;
				break;
			case 's':
				s1 = va_arg(ap, char *);
				while (minwidth-- > strlen(s1))
					sprocessed[j++] = ' ';
				while (s1[k])
					sprocessed[j++] = s1[k++];
				break;
			case 'd':
				number = va_arg(ap, int);
				intToAsc(number, buffer, 20);
				for (k = 0; k < 20; k++)
					if (20 - k > minwidth)
					{
						if (buffer[k] != ' ')
							sprocessed[j++] = buffer[k];
					}
					else
					{
						if (zeropadding && buffer[k] == ' ')
							buffer[k] = '0';
						sprocessed[j++] = buffer[k];
					}
				break;
			case 'x':
				;
				number = va_arg(ap, int);
				intToHAsc(number, buffer, 20);
				if (indicator)
				{
					sprocessed[j++] = '0';
					sprocessed[j++] = 'x';
				}
				for (k = 0; k < 20; k++)
					if (20 - k > minwidth)
					{
						if (buffer[k] != ' ')
							sprocessed[j++] = buffer[k];
					}
					else
					{
						if (zeropadding && buffer[k] == ' ')
							buffer[k] = '0';
						sprocessed[j++] = buffer[k];
					}
				break;
			default:
				break;
			}
			i++;
		}
	}
	va_end(ap);
	KWriteString(sprocessed, row, column);
	return 0;
}

char *strrchr(char *string, char delimiter)
{
	int n = strlen(string);
	int i;
	for (i = n - 1; i >= 0; i--)
		if (string[i] == delimiter)
			break;
	return string + i;
}

memset(char *address, char value, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		address[i] = value;
}

memcpy(char *dest, char *source, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		dest[i] = source[i];
}

long strspn(char *s1, char *s2)
{
	long ret = 0;
	while (*s1 && strchr(s2, *s1++))
		ret++;
	return ret;
}

long strcspn(char *s1, char *s2)
{
	long ret = 0;
	while (*s1)
		if (strchr(s2, *s1))
			return ret;
		else
			s1++, ret++;
	return ret;
}
char *strtok(char * str, char * delim)
{
	static char* p = 0;
	if (str)
		p = str;
	else if (!p)
		return 0;
	str = p + strspn(p, delim);
	p = str + strcspn(str, delim);
	if (p == str)
		return p = 0;
	p = *p ? *p = 0, p + 1 : 0;
	return str;
}

//==================================================================
// Finds first occurrence of c in string.
// Returns pointer to that occurrence or 0 if not found
//==================================================================
char *strchr(char *string, char c)
{
	long i = 0;
	while (string[i])
	{
		if (string[i] == c)
			return string + i;
		i++;
	}
	return 0;
}

//================================================================
// Returns the length of string
//================================================================
long strlen(char *string)
{
	int i = 0;
	while (string[i++])
		;
	return i - 1;
}

//===============================================================
// Compares s1 and s2 up to length characters.
// Returns 0 if they are equal, non-zero otherwise.
//===============================================================
long strncmp(char *s1, char *s2, size_t length)
{
	size_t count;
	short done = 0;

	for (count = 0; count < length; count++)
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

//=======================================================
// Copy null-terminates string s1 to s2
//=======================================================
char *strcpy(char *destination, char *source)
{
	char *retval = destination;
	while (*source)
	{
		*destination++ = *source++;
	}
	*destination = 0;
	return retval;
}

//===============================================================
// Compares s1 and s2.
// Returns 0 if they are equal, non-zero otherwise.
//===============================================================
long strcmp(char *s1, char *s2)
{
	if (s1 == s2) return 0;
	long retval = 1;
	int count = 0;
	while (s1[count] == s2[count])
	{
		if (s1[count++] == 0)
			retval = 0;
	}
	return retval;
}

//===========================================================
// Concatenates s2 to the end of s1.
// Returns s1
//===========================================================
char *strcat(char *s1, char *s2)
{
	int n = strlen(s1);
	strcpy(s1 + n, s2);
	return s1;
}

//=================================================================
// Convert the integer in i to its ASCII representation in buffer
// The integer is of length n
//=================================================================
int intToAsc(unsigned int i, char *buffer, int len)
{
	int count;

	for (count = 0; count < len; count++)
		buffer[count] = ' ';
	count = len - 1;
	do
	{
		buffer[count] = '0' + i % 10;
		i = i / 10;
		count--;
	} while (i > 0);
	return 0;
}

//=============================================================================
// Convert the integer in i to it's hesadecimal ASCII representation in buffer
// The integer is of length n
//=============================================================================
int intToHAsc(unsigned int i, char *buffer, int len)
{
	int count;

	for (count = 0; count < len; count++)
		buffer[count] = ' ';
	count = len - 1;
	do
	{
		if (i % 16 < 10)
			buffer[count] = '0' + i % 16;
		else
			buffer[count] = '0' + 7 + i % 16;
		i = i / 16;
		count--;
	} while (i > 0);
	return 0;
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
