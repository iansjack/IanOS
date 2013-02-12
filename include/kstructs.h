#ifndef KSTRUCTS_H
#define KSTRUCTS_H

typedef int FD;

#define	CONS	1
#define	KBD		2
#define FILE	3
#define DIR		4

#define STDIN	0
#define STDOUT	1
#define STDERR	2

struct Task
{
	long rax;
	long rbx;
	long rcx;
	long rdx;
	long rbp;
	long rsi;
	long rdi;
	long rsp;
	long r8;
	long r9;
	long r10;
	long r11;
	long r12;
	long r13;
	long r14;
	long r15;
	long rflags;
	long cr3;
	long firstdata;
	long firstfreemem;
	long timer;
	char *environment;
	struct MessagePort *parentPort;
	char *currentDirName;
	char **argv;
	long console;
	struct FCB *fcbList;
	long FDbitmap;
	unsigned short pid;
	short int ds;
	short int es;
	short int fs;
	short int gs;
	short int ss;
	unsigned char forking;
	unsigned char waiting;
};

struct Message
{
	struct Message *nextMessage;
	long quad1;
	long quad2;
	long quad3;
	struct MessagePort *tempPort;
	unsigned short pid;
	unsigned char byte;
};

struct MessagePort
{
	struct Task *waitingProc;
	struct Message *msgQueue;
};

struct MemStruct
{
	struct MemStruct *next;
	long size;
};

struct clusterListEntry
{
	struct clusterListEntry *next;
	unsigned short cluster;
};

struct Console
{
	char *kbBuffer;
	struct Message *MsgQueue;
	char *ConsoleBuffer;
	short kbBufStart;
	short kbBufCurrent;
	short kbBufCount;
	short row;
	short column;
	short colour;
};

#define SWTASKS		asm ("int $20")
#define SWTASKS15	asm ("mov %rdi, %r15"); asm ("int $22")
#define CLI			asm ("pushf"); asm("cli")
#define STI			asm ("popf")

#endif
