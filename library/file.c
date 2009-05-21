#include "cmemory.h"
#include "ckstructs.h"
#include "library/syscalls.h"
#include "filesystem.h"

struct FCB * CreateFile(char * s)
{
	char * S = sys_AllocSharedMem(12);
	char * str = S;
	while (*s != 0) *S++ = *s++;
	*S = 0;
	struct Message * msg = (struct Message *) sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = CREATEFILE;
	msg->quad = (long) str;
	SendReceive(FSPort, msg);
	return (struct FCB *) msg->quad;
}

struct FCB * OpenFile(char * s)
{
	char * S = sys_AllocSharedMem(12);
	char * str = S;
	while (*s != 0) *S++ = *s++;
	*S = 0;
	struct Message * msg = (struct Message *) sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = OPENFILE;
	msg->quad = (long) str;
	SendReceive(FSPort, msg);
	return (struct FCB *) msg->quad;
}

struct FCB * CloseFile(struct FCB * fHandle)
{
	struct Message * msg = (struct Message *) sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = CLOSEFILE;
	msg->quad = (long) fHandle;
	SendReceive(FSPort, msg);
	return (struct FCB *) msg->quad;
}

long ReadFile(struct FCB * fHandle, char * buffer, long noBytes)
{
	long retval;
    
	struct Message * FSMsg;
	FSMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	char * buff = sys_AllocSharedMem(noBytes);
	int i;

	FSMsg->nextMessage = 0;
	FSMsg->byte = READFILE;
	FSMsg->quad = (long) fHandle;
	FSMsg->quad2 = (long) buff;
	FSMsg->quad3 = noBytes;
	SendReceive(FSPort, FSMsg);
	for (i = 0; i < noBytes; i++) buffer[i] = buff[i];
	sys_DeallocMem(buff);
	retval = FSMsg->quad;
	sys_DeallocMem(FSMsg);
	return retval;
}
long WriteFile(struct FCB * fHandle, char * buffer, long noBytes)
{
	long retval;
    
	struct Message * FSMsg;
	FSMsg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	char * buff = sys_AllocSharedMem(noBytes);
	int i;
	for (i = 0; i < noBytes; i++) buff[i] = buffer[i];

	FSMsg->nextMessage = 0;
	FSMsg->byte = WRITEFILE;
	FSMsg->quad = (long) fHandle;
	FSMsg->quad2 = (long) buff;
	FSMsg->quad3 = noBytes;
	SendReceive(FSPort, FSMsg);
	sys_DeallocMem(buff);
	retval = FSMsg->quad;
	sys_DeallocMem(FSMsg);
	return retval;
}
