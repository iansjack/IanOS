#include "memory.h"
#include "kernel.h"
#include "console.h"

#define MAIN         1
#define PROCESS      2
#define PROCDETAILS  3

//====================================================
// This is the monitor task.
//====================================================
extern long Ticks;
extern unsigned char currentBuffer;
extern struct Task *currentTask;
extern struct Task *runnableTasks[2];  // [0] = Head, [1] = Tail
extern struct Task *blockedTasks[2];
extern struct Task *lowPriTask;
extern long NoOfAllocations;
extern long nextKPage;
extern long nPagesFree;
extern struct TaskList * allTasks;

char *mVideoBuffer;
struct Task *dispTask;
short printTemplate;

void mscrollscreen()
{
    short int row;
    short int column;

    for (row = 1; row < 25; row++)
        for (column = 0; column < 80; column++)
            mVideoBuffer[160 * (row - 1) + 2 * column] = mVideoBuffer[160 * row + 2 * column];
    for (column = 0; column < 80; column++)
        mVideoBuffer[160 * 24 + 2 * column] = ' ';
}

void mprintString(char *s)
{
    char *S   = (char *)AllocSharedMem(256);
    char *str = S;

    while (*s != 0)
    {
        *S++ = *s++;
    }
    *S = 0;
    struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));
    msg->nextMessage = 0;
    msg->byte        = WRITESTR;
    msg->quad        = (long)str;
    msg->quad2       = 1;
    SendMessage((struct MessagePort *)ConsolePort, msg);
    DeallocMem(S);
    DeallocMem(msg);
}


void mclrscr()
{
    struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));

    msg->nextMessage = 0;
    msg->byte        = CLRSCR;
    msg->quad        = 0;
    msg->quad2       = 1;
    SendMessage((struct MessagePort *)ConsolePort, msg);
    DeallocMem(msg);
}

void mclreol()
{
    struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));

    msg->nextMessage = 0;
    msg->byte        = CLREOL;
    msg->quad        = 0;
    msg->quad2       = 1;
    SendMessage((struct MessagePort *)ConsolePort, msg);
    DeallocMem(msg);
}
void msetcursor(long row, long column)
{
    struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));

    msg->nextMessage = 0;
    msg->byte        = SETCURSOR;
    msg->quad        = row;
    msg->quad2       = 1;
    msg->quad3       = column;
    SendMessage((struct MessagePort *)ConsolePort, msg);
    DeallocMem(msg);
}

void mprintchar(char c)
{
    struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));

    msg->nextMessage = 0;
    msg->byte        = WRITECHAR;
    msg->quad        = c;
    msg->quad2       = 1;
    SendMessage((struct MessagePort *)ConsolePort, msg);
    DeallocMem(msg);
}

void mprint64(unsigned long n)
{
    char nBuff[16];
    int i;
    for (i = 0; i < 16; i++)
    {
        char c = n % 16;
        if (c > 9) c +=7;
        c += '0';
        nBuff[15 - i] = c;
        n /= 16;
    }
    for (i = 8; i < 16; i++) mprintchar(nBuff[i]);
}

void mSetNormal()
{
    struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));

    msg->nextMessage = 0;
    msg->byte        = NORMAL;
    msg->quad        = 0;
    msg->quad2       = 1;
    SendMessage((struct MessagePort *)ConsolePort, msg);
    DeallocMem(msg);
}

void mSetReverse()
{
    struct Message *msg = (struct Message *)AllocKMem(sizeof(struct Message));

    msg->nextMessage = 0;
    msg->byte        = REVERSE;
    msg->quad        = 0;
    msg->quad2       = 1;
    SendMessage((struct MessagePort *)ConsolePort, msg);
    DeallocMem(msg);
}

unsigned char mgetkey()
{
    struct Message *kbdMsg;

    kbdMsg = (struct Message *)AllocKMem(sizeof(struct Message));
    kbdMsg->nextMessage = 0;
    kbdMsg->byte        = 3;
    kbdMsg->quad        = 1;
    SendReceiveMessage((struct MessagePort *)KbdPort, kbdMsg);
    char c = kbdMsg->byte;
    DeallocMem(kbdMsg);
    return(c);
}

void MainDisplay()
{
    struct Task *t;

    if (printTemplate)
    {
        mclrscr();
        mprintString("Monitor\r");
        mprintString("=======\r");
        mprintString("\rTicks:");
        mprintString("\rcurrentTask:");
        mprintString("\rrunnableTasks:");
        mprintString("\rblockedTasks:");
        mprintString("\r\rlowPriTask:");
        mprintString("\rFree Pages");
        mprintString("\rAllocations:");
    }

    msetcursor(3, 18);
    mprint64(Ticks);
    msetcursor(4, 18);
    mprint64((long) currentTask);
    msetcursor(5, 18);
    t = runnableTasks[0];
    while (t)
    {
        mprint64((long)t);
        t = t->nexttask;
        mprintchar (' ');
    }
    mclreol();
    msetcursor(6, 18);
    t = blockedTasks[0];
    while (t)
    {
        mprint64((long)t);
        t = t->nexttask;
        mprintchar (' ');
    }
    mclreol();
    msetcursor(8, 18);
    mprint64((long)lowPriTask);
    msetcursor(9, 18);
    mprint64((long)nPagesFree);
    msetcursor(10, 18);
    mprint64(NoOfAllocations);
}

void ProcessDisplay(int n /*struct Task * t*/)
{
    struct TaskList * tl = allTasks;
    struct Task * t;
    int count = 0;

    if (printTemplate)
    {
        mclrscr();
        mprintString("Processes\r");
        mprintString("=========\r");
    }
    msetcursor(3, 0);
    while (tl->next)
    {
        t = tl->task;
        mprint64(t->pid);
        mprintString("    ");
        if (count++ == n)
        {
            mSetReverse();
            mprint64((long)t);
            mSetNormal();
            dispTask = t;
        }
        else mprint64((long)t);
        tl = tl->next;
        mprintchar ('\r');
    }
    t = tl->task;
    mprint64(t->pid);
    mprintString("    ");
    if (count++ == n)
    {
        mSetReverse();
        mprint64((long)t);
        mSetNormal();
        dispTask = t;
    }
    else mprint64((long)t);
    mprintchar('\r');
    mclreol();
}

void ProcessDetails(/*struct Task * t*/)
{
    if (printTemplate)
    {
        mclrscr();
        mprintString("\rProcess:");
        mprintString("\rPID:");
        mprintString("\rNext Task:");
        msetcursor(1, 40);
        mprintString("RAX:");
        msetcursor(2, 40);
        mprintString("RBX:");
        msetcursor(3, 40);
        mprintString("RCX:");
        msetcursor(4, 40);
        mprintString("RDX:");
    }
    msetcursor(1, 14);
    mprint64((long)dispTask);
    msetcursor(2, 14);
    mprint64((long)dispTask->pid);
    msetcursor(3, 14);
    mprint64((long)dispTask->nexttask);
    msetcursor(1, 48);
    mprint64(dispTask->rax);
    msetcursor(2, 48);
    mprint64(dispTask->rbx);
    msetcursor(3, 48);
    mprint64(dispTask->rcx);
    msetcursor(4, 48);
    mprint64(dispTask->rdx);
}

void monitorTaskCode()
{
    struct Task * t;

    int display = MAIN;
    int taskno = 0;
    printTemplate = 1;

    while (1)
    {
        switch (display)
        {
        case MAIN:
            MainDisplay();
            break;
        case PROCESS:
            ProcessDisplay(taskno);
            break;
        case PROCDETAILS:
            ProcessDetails(t);
            break;
        }
        GoToSleep(20);
        char c = mgetkey();
        printTemplate = 1;
        switch (c)
        {
        case 'm':
            display = MAIN;
            break;
        case 'b':
            display = PROCESS;
            break;
        case 'n':
            switch (display)
            {
            case PROCESS:
                taskno++;
                break;
            default:
                break;
            }
            break;
        case 'p':
            switch (display)
            {
            case PROCESS:
                if (taskno) taskno--;
                break;
            default:
                break;
            }
            break;
        case 'd':
            switch (display)
            {
            case PROCESS:
                display = PROCDETAILS;
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


