#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

//#undef __mode_t
//typedef mode_t __mode_t;

//#undef __ino_t
//typedef ino_t __ino_t;

# ifdef S_SPLINT_S
struct timeval
  {
    __time_t tv_sec;		/* Seconds.  */
    __suseconds_t tv_usec;	/* Microseconds.  */
  };
#endif

typedef unsigned char	uint_8;
typedef unsigned short	uint_16;
typedef unsigned int	uint_32;
typedef unsigned long	uint_64;

#endif /* LINUX_TYPES_H */
