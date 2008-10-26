#ifndef CKSTRUCTS_H
#define CKSTRUCTS_H

struct Task
{
	struct Task * nexttask;
	unsigned char waiting;
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
	short int ds;
	short int es;
	short int fs;
	short int gs;
	short int ss;
	long cr3;
	long firstdata;
	long firstfreemem;
	long nextpage;
	long pid;
};

struct Message
{
	struct Message * nextMessage;
	unsigned char byte;
	long quad;
};

struct MessagePort
{
	struct Task * waitingProc;
	struct Message * msgQueue;
};

struct MemStruct
{
	struct MemStruct * next;
	long size;
};

struct FCB
{
	int	 length;
	long startSector;
	long nextSector;
	long fileCursor;
	long bufCursor;
	char * filebuf;
};

#endif
