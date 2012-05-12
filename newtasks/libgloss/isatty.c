/*
 * Stub version of isatty.
 */
extern int errno;

int isatty(int file)
{
//  errno = -1 ; // ENOSYS;
  return 1;
}

