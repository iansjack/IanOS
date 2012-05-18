#include <errno.h>
#include "../../include/memory.h"

extern int errno;

static char sbrk_first_time = 1;
static int sbrk_size = 0;
static void * sbrk_curbrk;

void *sbrk(int size)
{
	if (sbrk_first_time) 
	{
		long *firstfreemem = (long *)(UserCode + 22);
		sbrk_first_time = 0;
		sbrk_size = 0x10;
		sbrk_curbrk = (void *)(UserData + firstfreemem);
		return sbrk_curbrk;
	}
	
	if (size <= 0) return(sbrk_curbrk);
	while (size > sbrk_size) {
	    Alloc_Page(sbrk_curbrk + sbrk_size + 10);
	    sbrk_size += 0x1000;
	}
	sbrk_curbrk += size;
	sbrk_size -= size;
	return((void *)(sbrk_curbrk - size));
}

void *brk(void *x)
{
	errno = ENOMEM;
	return((void *)-1);
}


