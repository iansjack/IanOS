/*
 * Stub version of isatty.
 */
//#include <errno.h>
//extern int errno;

int isatty(int file)
{
//  errno = -1 ; // ENOSYS;
  return 1;
}

