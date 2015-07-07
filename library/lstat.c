
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <types.h>
#include "filesystem.h"

int sys_lstat(char *, struct FileInfo *);

#undef __mode_t
typedef mode_t __mode_t;

#undef __ino_t
typedef ino_t __ino_t;


#undef errno
extern int errno;

int lstat(char *path, struct stat *st)
{
	struct FileInfo inf;
	sys_lstat(path, &inf);
	st->st_mode = (__mode_t) S_IFCHR;
	st->st_ino = (__ino_t) inf.inode;
	st->st_mode = (__mode_t) inf.mode;
	st->st_uid = (__uid_t) inf.uid;
	st->st_gid = (__gid_t) inf.gid;
  	st->st_size = inf.size;
  	st->st_atime = inf.atime;
  	st->st_ctime = inf.ctime;
  	st->st_mtime = inf.mtime;
  	errno = 0;
  	return 0;
}
