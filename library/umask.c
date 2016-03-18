#include <sys/types.h>
#include <sys/stat.h>

#undef __mode_t
typedef mode_t __mode_t;

#undef __ino_t
typedef ino_t __ino_t;


mode_t umask(__mode_t mask)
{
	return 0;
}
