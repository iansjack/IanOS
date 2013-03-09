#include <errno.h>
#include <kernel.h>

#undef errno

void * Alloc_Page(void *);

extern int errno;

static char sbrk_first_time = 1;
static int sbrk_size = 0;
static void * sbrk_curbrk;

void *sbrk(int size)
{
	if (sbrk_first_time) 
	{
		sbrk_first_time = 0;
		sbrk_curbrk = Alloc_Page(0);
		sbrk_size = PageSize - ((long) sbrk_curbrk % PageSize);
		while (sbrk_size < 0)
			sbrk_size += PageSize;
	}
	
	if (size <= 0) return(sbrk_curbrk);
	while (size > sbrk_size) {
	    if (Alloc_Page(sbrk_curbrk + sbrk_size))
	    	sbrk_size += PageSize;
	    else
	    {
	    	errno = ENOMEM;
	    	return ((void *) -1);
	    }
	}
	sbrk_curbrk += size;
	sbrk_size -= size;
	errno = 0;
	return((void *)(sbrk_curbrk - size));
}

void *brk(void *x)
{
	errno = ENOMEM;
	return((void *)-1);
}


