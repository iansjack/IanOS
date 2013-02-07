#include <errno.h>
#include <kernel.h>

#undef errno

void * Alloc_Page(void *);

extern int errno;
// extern long DataLen;

static char sbrk_first_time = 1;
static int sbrk_size = 0;
static void * sbrk_curbrk;

void *sbrk(int size)
{
	if (sbrk_first_time) 
	{
		sbrk_first_time = 0;
		// sbrk_curbrk = (void *)(UserData + DataLen); //*firstfreemem);
		sbrk_curbrk = Alloc_Page(0);
		// sbrk_size = PageSize - DataLen;
		sbrk_size = PageSize - (long) sbrk_curbrk;
		while (sbrk_size < 0)
			sbrk_size += PageSize;
	}
	
	if (size <= 0) return(sbrk_curbrk);
	while (size > sbrk_size) {
	    Alloc_Page(sbrk_curbrk + sbrk_size + 10);
	    sbrk_size += PageSize;
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


