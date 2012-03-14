#ifndef KSTRUCTS_H
#define KSTRUCTS_H

typedef int	FD;

#define	CONS	1
#define	KBD		2
#define FILE	3
#define DIR		4

#define STDIN	1
#define STDOUT	2
#define STDERR	3

struct Task
{
    struct Task * nexttask;
    unsigned char waiting;
    long          rax;
    long          rbx;
    long          rcx;
    long          rdx;
    long          rbp;
    long          rsi;
    long          rdi;
    long          rsp;
    long          r8;
    long          r9;
    long          r10;
    long          r11;
    long          r12;
    long          r13;
    long          r14;
    long          r15;
    long          rflags;
    short int     ds;
    short int     es;
    short int     fs;
    short int     gs;
    short int     ss;
    long          cr3;
    long          firstdata;
    long          firstfreemem;
    long          nextpage;
    unsigned short pid;
    long          timer;
    char * 		  environment;
    struct MessagePort * parentPort;
    unsigned char	*currentDirName;
	unsigned char  	**argv;
    long          	console;
	unsigned char 	forking;
	struct FCB *  	fcbList;
};

struct Message
{
    struct Message *nextMessage;
    unsigned char  byte;
    long           quad;
    long           quad2;
    long           quad3;
    long           tempPort;
    long           pid;
};

struct MessagePort
{
    struct Task    *waitingProc;
    struct Message *msgQueue;
};

struct MemStruct
{
    struct MemStruct *next;
    long             size;
    long             pid;
};

struct clusterListEntry
{
    struct clusterListEntry *next;
    unsigned short          cluster;
};

struct FCB
{
    struct DirEntry *directory;
    unsigned int    length;
    unsigned long   startSector;
    unsigned long   nextSector;
    unsigned long   sectorInCluster;
    unsigned short  currentCluster;
    unsigned short  startCluster;
    unsigned long   fileCursor;
    unsigned short  bufCursor;
    unsigned char   bufIsDirty;          // 0 = clean, 1 = dirty
	unsigned char	deviceType;
    unsigned char   *filebuf;
	FD				fileDescriptor;
    long            pid;
	struct FCB *	nextFCB;
};

struct Console
{
  unsigned char * kbBuffer;
  short kbBufStart;
  short kbBufCurrent;
  short kbBufCount;
  struct Message * MsgQueue;
  unsigned char * ConsoleBuffer;
  short row;
  short column;
  short colour;
};


#define SWTASKS      asm ("int $20")
#define SWTASKS15    asm ("mov %rdi, %r15"); asm ("int $22")
#define CLI			asm ("pushf"); asm("cli")
#define STI			asm ("popf")

#endif
