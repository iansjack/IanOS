#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define CREATEFILE  	1
#define OPENFILE    	2
#define CLOSEFILE   	3
#define READFILE    	4
#define WRITEFILE   	5
#define DELETEFILE  	6
#define TESTFILE  		7
#define GETFILEINFO 	8
#define CREATEDIR		9
#define SEEK			10
#define TRUNCATE		11

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

struct DirectoryBuffer
{
	long Directory; // The cluster number of the directory
	void *Buffer; // A buffer containing the directory
};

struct FileInfo
{
	long inode;
	long mode;
	long uid;
	long gid;
	long size;
	long atime;
	long mtime;
	long ctime;
};

#endif
