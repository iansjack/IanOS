#include "memory.h"
#include "kstructs.h"
#include "filesystem.h"
#include "syscalls.h"
#include "fat.h"

long GetDirectory(char *directory)
{
	char *S = sys_AllocSharedMem(12);
	char *str = S;

	while (*directory != 0) {
		*S++ = *directory++;
	}
	*S = 0;
	struct Message *msg =
	    (struct Message *)sys_AllocMem(sizeof(struct Message));
	msg->nextMessage = 0;
	msg->byte = GETDIRECTORY;
	msg->quad = (long)str;
	sys_SendReceive(FSPort, msg);
	sys_DeallocMem(S);
	return (msg->quad);
}
