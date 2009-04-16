#include "cmemory.h"
#include "ckstructs.h"
#include "library/syscalls.h"
#include "console.h"

void getsector(long sector, char * buffer)
{
	char * B = sys_AllocSharedMem(512);
	struct Message * msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = 1;
	msg->quad2 = sector;
	msg->quad3 = (long) B;
	SendReceive(FSPort, msg);
	int i;
	for (i == 0; i < 512; i++) *buffer++ = *B++;
	sys_DeallocMem(B);
	sys_DeallocMem(msg);
}
    
void writeconsolechar(char c)
{
	struct Message * msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = WRITECHAR;
	msg->quad = c;
	sys_SendMessage(ConsolePort, msg);
	sys_DeallocMem(msg);
}

void writeconsolestring(char * s)
{
	char * S = sys_AllocSharedMem(256);
	char * str = S;
	while (*s != 0) *S++ = *s++;
	*S = 0;
	struct Message * msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = WRITESTR;
	msg->quad = (long)str;
	sys_SendMessage(ConsolePort, msg);
	//sys_DeallocMem(S);
	sys_DeallocMem(msg);
}

void consoleclrscr()
{
	struct Message * msg = (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = CLRSCR;
	msg->quad = 0;
	sys_SendMessage(ConsolePort, msg);
	sys_DeallocMem(msg);
}
