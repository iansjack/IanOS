#include "memory.h"
#include "kernel.h"
#include "console.h"
#include "kstructs.h"
#include "tasklist.h"

#define MAIN         	1
#define PROCESS      	2
#define PROCDETAILS  	3
#define MEMORY			4

//====================================================
// This is the monitor task.
//====================================================
extern long Ticks;
extern struct Task *currentTask;
extern struct Task *runnableTasks[2];	// [0] = Head, [1] = Tail
extern struct Task *blockedTasks[2];
extern struct Task *lowPriTask;
extern long nPagesFree;
extern long nPages;
extern struct TaskList *allTasks;

char *mVideoBuffer;
struct Task *dispTask;
short printTemplate;
unsigned long *baseMemory = (unsigned long *)0x310000;
long MonitorConsole = 3;

void mscrollscreen()
{
	mPrintChar(ESC);
	mPrintString("%c[2J");
}

void mPrintString(char *s)
{
	char *S = (char *)AllocKMem(256);
	char *str = S;

	while (*s != 0) {
		*S++ = *s++;
	}
	*S = 0;
	struct Message *msg = (struct Message *)ALLOCMSG;
	msg->nextMessage = 0;
	msg->byte = WRITESTR;
	msg->quad = (long)str;
	msg->quad2 = MonitorConsole;
	SendMessage((struct MessagePort *)ConsolePort, msg);
	DeallocMem(S);
	DeallocMem(msg);
}

void mclrscr()
{
	mPrintChar(ESC);
	mPrintString("%cD");
}

void mclreol()
{
	mPrintChar(ESC);
	mPrintString("%c[0K");
}

void msetcursor(long row, long column)
{
	mPrintChar(ESC);
	mPrintChar('[');
	if (row > 10) {
		mPrintChar(row / 10 + '0');
		mPrintChar(row % 10 + '0');
	} else
		mPrintChar(row + '0');
	mPrintChar(';');
	if (column > 10) {
		mPrintChar(column / 10 + '0');
		mPrintChar(column % 10 + '0');
	} else
		mPrintChar(column + '0');
	mPrintChar('H');
}

void mPrintChar(char c)
{
	struct Message *msg = (struct Message *)ALLOCMSG;

	msg->nextMessage = 0;
	msg->byte = WRITECHAR;
	msg->quad = c;
	msg->quad2 = MonitorConsole;
	SendMessage((struct MessagePort *)ConsolePort, msg);
	DeallocMem(msg);
}

void mprint64(unsigned long n)
{
	char nBuff[16];
	int i;
	for (i = 0; i < 16; i++) {
		char c = n % 16;
		if (c > 9)
			c += 7;
		c += '0';
		nBuff[15 - i] = c;
		n /= 16;
	}
	for (i = 0; i < 16; i++)
		mPrintChar(nBuff[i]);
}

void mSetNormal()
{
	mPrintChar(ESC);
	mPrintString("%c[?5l");
}

void mSetReverse()
{
	mPrintChar(ESC);
	mPrintString("%c[?5h");
}

unsigned char mgetkey()
{
	struct Message *kbdMsg;

	kbdMsg = (struct Message *)ALLOCMSG;
	kbdMsg->nextMessage = 0;
	kbdMsg->byte = 3;
	kbdMsg->quad = MonitorConsole;
	SendReceiveMessage((struct MessagePort *)KbdPort, kbdMsg);
	char c = kbdMsg->byte;
	DeallocMem(kbdMsg);
	return (c);
}

void MainDisplay()
{
	struct Task *t;
	mPrintChar(ESC);
	mPrintString("#3");

	if (printTemplate) {
		mclrscr();
		mPrintString("Monitor\n");
		mPrintString("=======\n");
		mPrintString("\nTicks:");
		mPrintString("\ncurrentTask:");
		mPrintString("\nrunnableTasks:");
		mPrintString("\nblockedTasks:");
		mPrintString("\n\nlowPriTask:");
		mPrintString("\nTotalPages");
		mPrintString("\nFree Pages");
		mPrintString("\nAllocations:");
	}

	msetcursor(3, 18);
	mprint64(Ticks);
	msetcursor(4, 18);
	mprint64((long)currentTask);
	msetcursor(5, 18);
	t = runnableTasks[0];
	while (t) {
		mprint64((long)t);
		t = t->nexttask;
		mPrintChar(' ');
	}
	mclreol();
	msetcursor(6, 18);
	t = blockedTasks[0];
	while (t) {
		mprint64((long)t);
		t = t->nexttask;
		mPrintChar(' ');
	}
	mclreol();
	msetcursor(8, 18);
	mprint64((long)lowPriTask);
	msetcursor(9, 18);
	mprint64(nPages);
	msetcursor(10, 18);
	mprint64(nPagesFree);
	msetcursor(11, 18);
//   mprint64(NoOfAllocations);
}

void ProcessDisplay(int n)
{
	struct TaskList *tl = allTasks;
	struct Task *t;
	int count = 0;

	if (printTemplate) {
		mclrscr();
		mPrintString("Processes\n");
		mPrintString("=========\n");
	}
	msetcursor(3, 0);
	while (tl->next) {
		t = tl->task;
		mprint64(t->pid);
		mPrintString("    ");
		if (count++ == n) {
			mSetReverse();
			mprint64((long)t);
			mSetNormal();
			dispTask = t;
		} else
			mprint64((long)t);
		tl = tl->next;
		mPrintChar('\n');
	}
	t = tl->task;
	mprint64(t->pid);
	mPrintString("    ");
	if (count++ == n) {
		mSetReverse();
		mprint64((long)t);
		mSetNormal();
		dispTask = t;
	} else
		mprint64((long)t);
	mPrintChar('\n');
	mclreol();
}

void ProcessDetails()
{
	if (printTemplate) {
		mclrscr();
		mPrintString("\nProcess:");
		mPrintString("\nPID:");
		mPrintString("\nNext Task:");
		mPrintString("\nFirstFreeMem:");
		mPrintString("\nCurrentDir:");
		msetcursor(1, 40);
		mPrintString("RAX:");
		msetcursor(2, 40);
		mPrintString("RBX:");
		msetcursor(3, 40);
		mPrintString("RCX:");
		msetcursor(4, 40);
		mPrintString("RDX:");
		msetcursor(5, 40);
		mPrintString("RSI:");
		msetcursor(6, 40);
		mPrintString("RDI:");
		msetcursor(7, 40);
		mPrintString("RBP:");
		msetcursor(8, 40);
		mPrintString("RSP:");
		msetcursor(9, 40);
		mPrintString("R8 :");
		msetcursor(10, 40);
		mPrintString("R9 :");
		msetcursor(11, 40);
		mPrintString("R10:");
		msetcursor(12, 40);
		mPrintString("R11:");
		msetcursor(13, 40);
		mPrintString("R12:");
		msetcursor(14, 40);
		mPrintString("R13:");
		msetcursor(15, 40);
		mPrintString("R14:");
		msetcursor(16, 40);
		mPrintString("R15:");
	}
	msetcursor(1, 14);
	mprint64((long)dispTask);
	msetcursor(2, 14);
	mprint64((long)dispTask->pid);
	msetcursor(3, 14);
	mprint64((long)dispTask->nexttask);
	msetcursor(4, 14);
	mprint64((long)dispTask->firstfreemem);
	msetcursor(5, 14);
	mPrintString(dispTask->currentDirName);
	msetcursor(1, 48);
	mprint64(dispTask->rax);
	msetcursor(2, 48);
	mprint64(dispTask->rbx);
	msetcursor(3, 48);
	mprint64(dispTask->rcx);
	msetcursor(4, 48);
	mprint64(dispTask->rdx);
	msetcursor(5, 48);
	mprint64(dispTask->rsi);
	msetcursor(6, 48);
	mprint64(dispTask->rdi);
	msetcursor(7, 48);
	mprint64(dispTask->rbp);
	msetcursor(8, 48);
	mprint64(dispTask->rsp);
	msetcursor(9, 48);
	mprint64(dispTask->r8);
	msetcursor(10, 48);
	mprint64(dispTask->r9);
	msetcursor(11, 48);
	mprint64(dispTask->r10);
	msetcursor(12, 48);
	mprint64(dispTask->r11);
	msetcursor(13, 48);
	mprint64(dispTask->r12);
	msetcursor(14, 48);
	mprint64(dispTask->r13);
	msetcursor(15, 48);
	mprint64(dispTask->r14);
	msetcursor(16, 48);
	mprint64(dispTask->r15);
}

void printLongData(long data)
{
	long value;

 asm("mov %%cr3,%%rdi;" "mov %1,%%rax;" "mov %%rax,%%cr3;" "mov %2,%%rax;" "mov (%%rax),%%rax;" "mov %%rax,%0;" "mov %%rdi,%%cr3;": "=r"(value): "r"(dispTask->cr3), "r"(data):"%rax",
	    "%rdi");
	mprint64(value);
}

void printCharData(long data)
{
	char value;

 asm("mov %%cr3,%%rdi;" "mov %1,%%rax;" "mov %%rax,%%cr3;" "mov %2,%%rax;" "mov (%%rax),%%rax;" "mov %%al,%0;" "mov %%rdi,%%cr3;": "=r"(value): "r"(dispTask->cr3), "r"(data):"%rax",
	    "%rdi");
	if (value < ' ')
		value = '.';
	mPrintChar(value);
}

void ProcessMemory()
{
	unsigned long *data = baseMemory;
	int i, j, k;

	if (printTemplate) {
		mclrscr();
		mPrintString("Process Memory\n");
		mPrintString("==============\n");
	}

	for (i = 0; i < 8; i++) {
		msetcursor(3 + i, 1);
		mprint64((unsigned long)data);
		mPrintChar(':');
		mPrintChar(' ');
		for (j = 0; j < 2; j++) {
			printLongData((long)(data + j));
			mPrintChar(' ');
		}
		for (j = 0; j < 2; j++) {
			mPrintChar(' ');
			for (k = 0; k < 8; k++)
				printCharData((long)
					      ((unsigned char *)data + k));
			data++;
		}
	}
}

void monitorTaskCode()
{
	struct Task *t;

	int display = MAIN;
	int taskno = 0;
	printTemplate = 1;

	while (1) {
		switch (display) {
		case MAIN:
			MainDisplay();
			break;
		case PROCESS:
			ProcessDisplay(taskno);
			break;
		case PROCDETAILS:
			ProcessDetails(t);
			break;
		case MEMORY:
			ProcessMemory(t);
			break;
		}
		GoToSleep(20);
		char c = mgetkey();
		printTemplate = 1;
		switch (c) {
		case 'h':
			display = MAIN;
			break;
		case 'b':
			display = PROCESS;
			break;
		case 'n':
			switch (display) {
			case PROCESS:
				taskno++;
				break;
			case MEMORY:
				baseMemory += 16;
				break;
			default:
				break;
			}
			break;
		case 'p':
			switch (display) {
			case PROCESS:
				if (taskno)
					taskno--;
				break;
			case MEMORY:
				if (baseMemory > (unsigned long *)UserData)
					baseMemory -= 16;
				break;
			default:
				break;
			}
			break;
		case 'd':
			switch (display) {
			case MEMORY:
			case PROCESS:
				display = PROCDETAILS;
				break;
			default:
				break;
			}
			break;
		case 'm':
			switch (display) {
			case PROCDETAILS:
			case PROCESS:
				display = MEMORY;
				break;
			default:
				break;
			}
			break;
		default:
			printTemplate = 0;
			break;
		}
	}
}
