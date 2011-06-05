#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define CREATEFILE   1
#define OPENFILE     2
#define CLOSEFILE    3
#define READFILE     4
#define WRITEFILE    5
#define DELETEFILE   6
#define GETPID       7
#define GETDIRENTRY  8
#define GETDIRECTORY 9

struct DirectoryBuffer
{
   long Directory;   // The cluster number of the directory
   void * Buffer;    // A buffer containing the directory
};

unsigned char   *DiskBuffer;
unsigned short  *FAT;

#endif
