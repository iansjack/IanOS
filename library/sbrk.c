#include <errno.h>
#include <kernel.h>

l_Address Alloc_Page(unsigned char);

l_Address sbrk(int size)
{
	l_Address sbrk_curbrk = Alloc_Page(0);
	if (size <= 0) return(Alloc_Page(0));
	
	long sbrk_size = 0;
	while (size > sbrk_size) {
	    if (Alloc_Page(1))
	    	sbrk_size += PageSize;
	    else
	    {
	    	errno = ENOMEM;
	    	return ((l_Address) -1);
	    }
	}
	errno = 0;
	return(sbrk_curbrk);
}

l_Address brk(l_Address x)
{
	errno = ENOMEM;
	return((l_Address)-1);
}


