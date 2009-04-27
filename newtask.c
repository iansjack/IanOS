#include "ckstructs.h"
#include "cmemory.h"
#include "filesystem.h"

struct Task * currentTask;
struct Task * runnableTasksHead;
struct Task * runnableTasksTail;
struct Task * blockedTasksHead;
struct Task * blockedTasksTail;
struct Task * lowPriTask;

extern long * tempstack;
extern long * tempstack0;
extern unsigned char * PMap;
extern long nPagesFree;

static long nextpid = 2;

long ReadFromFile(struct FCB * fHandle, char * buffer, long noBytes)
{
	long retval;
    
	struct Message * FSMsg;
	FSMsg = (struct Message *)AllocKMem(sizeof(struct Message));
	char * buff = AllocKMem(noBytes);
	int i;
	for (i = 0; i < noBytes; i++) buff[i] = buffer[i];

	FSMsg->nextMessage = 0;
	FSMsg->byte = READFILE;
	FSMsg->quad = (long) fHandle;
	FSMsg->quad2 = (long) buff;
	FSMsg->quad3 = noBytes;
	SendReceiveMessage(FSPort, FSMsg);
	for (i = 0; i < noBytes; i++) buffer[i] = buff[i];
	DeallocMem(buff);
	retval = FSMsg->quad;
	DeallocMem(FSMsg);
	return retval;
}

//============================================
//  Find the next free entry in the task table
//============================================

struct Task * nextfreetss()
{
	struct Task * temp = (struct Task *)TaskStruct;
	while (temp->pid != 0)
		temp++;
	return temp;
}

//===============================
// Link task into the task table
// Also allocate a pid
//===============================

void LinkTask(struct Task * task)
{
	task->nexttask = runnableTasksHead;
	runnableTasksHead = task;
	task->pid = nextpid++;
}

//========================
// Create a new User task
//========================

void NewTask(char * name)
{
	long * stack;
	struct Task * task = nextfreetss();
	long * data;
	struct FCB * fHandle;
	long codelen, datalen;
	char header[3];
	int result;
	struct Message * FSMsg;

	fHandle = (struct FCB *)AllocKMem(sizeof(struct FCB));
	FSMsg = (struct Message *)AllocKMem(sizeof(struct Message));

	// Open file
	FSMsg->nextMessage = 0;
	FSMsg->byte = OPENFILE;
	FSMsg->quad = (long) name;
	FSMsg->quad2 = (long) fHandle;
	SendReceiveMessage(FSPort, FSMsg);
    
	if (FSMsg->quad == 0)
	{
		stack = (long *) &tempstack - 5;
		task->rsp = (long) stack;
		task->r15 = (long) task;
		stack[0] = UserCode;
		stack[1] = user64 + 3;
		stack[2] = 0x2202;
		stack[3] = UserData + PageSize;
		stack[4] = udata64 + 3;
		task->waiting = 0;
		task->cr3 = VCreatePageDir();
		task->ds = udata64 + 3;
		ReadFromFile(fHandle, header, 4);
		ReadFromFile(fHandle, (char *) &codelen, 8);
		ReadFromFile(fHandle, (char *) &datalen, 8);
		ReadFromFile(fHandle, (char *) TempUserCode, codelen);
		ReadFromFile(fHandle, (char *) TempUserData, datalen);
		data = (long *) (TempUserData + datalen);
		data[0] = 0;
		data[1] = PageSize - datalen - 0x10;
		task->firstfreemem = UserData + datalen;
		
		//Close file
		FSMsg->nextMessage = 0;
		FSMsg->byte = CLOSEFILE;
		FSMsg->quad = (long) fHandle;
		SendReceiveMessage(FSPort, FSMsg);
	    
		asm("cli");
		LinkTask(task);
		asm("sti");
	}
	DeallocMem(fHandle);
	DeallocMem(FSMsg);
}

//==========================
// Create a new Kernel task
//==========================

struct Task * NewKernelTask(void * TaskCode)
{
	long * stack;
	struct Task * task = nextfreetss();
	long * data;

	stack = (long *)&tempstack - 5;
	task->rsp = (long) stack;
	task->r15 = (long) task;
	stack[0] = (long) TaskCode;
	stack[1] = code64;
	stack[2] = 0x2202;
	stack[3] = (long) &tempstack0;
	stack[4] = data64;
	task->waiting = 0;
	task->cr3 = VCreatePageDir();
	task->ds = data64;
	asm("cli");
	LinkTask(task);
	data = (long *) TempUserData;
	data[0] = 0;
	data[1] = 0xFFE;
	task->firstfreemem = UserData;
	asm("sti");
}

//=============================
// Create the low-priority task
//=============================

void NewLowPriTask(void * TaskCode)
{
	lowPriTask = NewKernelTask(TaskCode);
	struct Task * temp = runnableTasksHead;

	// Unlink task from runnable queue
	if (temp == lowPriTask) 
	{
		runnableTasksHead = temp->nexttask;
		if (runnableTasksHead == 0) runnableTasksTail = 0;
	}
	else
		while (temp)
		{
			if (temp->nexttask == lowPriTask) temp->nexttask = temp->nexttask->nexttask;
			if (temp->nexttask == 0) runnableTasksTail = temp;
			temp = temp->nexttask;
		}
}

//=======================
// Kill the current task
//=======================

void KillTask(void)
{
	struct Task * task = currentTask;
	struct Task * temp = runnableTasksHead;

	//Don't want to task switch whilst destroying task
	asm("cli");
	
	// Unlink task from runnable queue
	if (temp == task) 
	{
		runnableTasksHead = temp->nexttask;
		if (runnableTasksHead == 0) runnableTasksTail = 0;
	}
	else
		while (temp)
		{
			if (temp->nexttask == task) temp->nexttask = temp->nexttask->nexttask;
			if (temp->nexttask == 0) runnableTasksTail = temp;
			temp = temp->nexttask;
		}
	
	// Release allocated memory
	long * mem = (long *)PageTableL12;
	long count;
	for (count = 0x0; count < 0x3; count++)
	{
		PMap[mem[count] >> 12] = 0;
		nPagesFree++;
	}
	for (count = 0x4; count < 0x200; count++)
	{
		if (mem[count] != 0 )
		{
			PMap[mem[count] >> 12] = 0;
			nPagesFree++;
		}
	}
	SwTasks();
}

//===============================================
// Move task from runnable queue to blocked queue
//===============================================

void BlockTask(struct Task * task)
{
	struct Task * temp = runnableTasksHead;

	// Unlink task from runnable queue
	if (temp == task) 
	{
		runnableTasksHead = temp->nexttask;
		if (runnableTasksHead == 0) runnableTasksTail = 0;
	}
	else
		while (temp)
		{
			if (temp->nexttask == task) temp->nexttask = temp->nexttask->nexttask;
			if (temp->nexttask == 0) runnableTasksTail = temp;
			temp = temp->nexttask;
		}

	// Put task on to blocked queue
	if (blockedTasksHead == 0)
	{
		blockedTasksHead = blockedTasksTail = task;
		task->nexttask = 0;
	}
	else
	{
		blockedTasksTail->nexttask = task;
		blockedTasksTail = task;
		task->nexttask = 0;
	}
}

//===============================================
// Move task from blocked queue to runnable queue
//===============================================

void UnBlockTask(struct Task * task)
{
	struct Task * temp = blockedTasksHead;

	// Unlink task from blocked queue
	if (temp == task) 
		blockedTasksHead = temp->nexttask;
	else
		while (temp)
		{
			if (temp->nexttask == task) temp->nexttask = temp->nexttask->nexttask;
			if (temp->nexttask == 0) blockedTasksTail = temp;
			temp = temp->nexttask;
		}

	// Put task on to runnable queue
	if (runnableTasksHead == 0)
	{
		runnableTasksHead = runnableTasksTail = task;
		task->nexttask = 0;
	}
	else
	{
		runnableTasksTail->nexttask = task;
		runnableTasksTail = task;
		task->nexttask = 0;
	}
}

//===========================================================
// Move the task at the head of the runnable queue to the end
//===========================================================

void moveTaskToEndOfQueue()
{
	struct Task * temp = runnableTasksHead;
	if (temp->nexttask)
	{
		runnableTasksHead = temp->nexttask;
		runnableTasksTail->nexttask = temp;
		runnableTasksTail = temp;
		temp->nexttask = 0;
	}
}

//=====================================
// The one task that is always runnable
//=====================================

void dummyTask()
{
	CreatePTE(AllocPage64(), KernelStack);
	CreatePTE(AllocPage64(), UserStack);
	asm("mov $(0x3FF000 - 0x18), %rsp");

	while (1)
	{
	}
}
