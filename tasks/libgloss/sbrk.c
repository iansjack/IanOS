#include <errno.h>

extern int errno;

static char sbrk_first_time = 1;
static int sbrk_size = 0;
static void * sbrk_curbrk;

void *sbrk(int size)
{
	if (sbrk_first_time) 
	{
		sbrk_first_time = 0;
		sbrk_size = 0x10;
		sbrk_curbrk = (void *)0x601FF0;
		return sbrk_curbrk;
	}
	
	if (size <= 0) return(sbrk_curbrk);
	while (size > sbrk_size) {
//		errno = ENOMEM;
//		return((void *)-1);
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


