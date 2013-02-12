#include <sys/types.h>
#include <sys/stat.h>
#include <types.h>

//#undef __mode_t
//typedef mode_t __mode_t;

mode_t umask(__mode_t mask)
{
	return 0;
}
