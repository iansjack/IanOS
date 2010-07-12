#include "memory.h"
#include "kernel.h"
#include "console.h"

//====================================================
// This is the monitor task.
//====================================================
extern long Ticks;
extern unsigned char currentBuffer;
extern struct Task *currentTask;
extern struct Task *runnableTasks[2];  // [0] = Head, [1] = Tail
extern struct Task *blockedTasks[2];
extern struct Task *lowPriTask;

char *mVideoBuffer;

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

void mprint64(long n)
{
    char nBuff[8];
    int i;
    for (i = 0; i < 8; i++)
    {
        char c = n % 16;
        if (c > 9) c +=7;
        c += '0';
        nBuff[7 - i] = c;
        n /= 16;
    }
    for (i = 0; i < 8; i++) mprintchar(nBuff[i]);
}

unsigned char mgetkey()
{
    struct Message *kbdMsg;

    kbdMsg = (struct Message *)AllocKMem(sizeof(struct Message));
    kbdMsg->nextMessage = 0;
    kbdMsg->byte        = 3;
    kbdMsg->quad        = 1;
    SendReceiveMessage(KbdPort, kbdMsg);
    char c = kbdMsg->byte;
    DeallocMem(kbdMsg);
    return(c);
}

void monitorTaskCode()
{
    struct Task * t;

    mclrscr();
    mprintString("Monitor\r");
    mprintString("=======\r");
    mprintString("\rTicks:         ");
    mprintString("\rcurrentTask:   ");
    mprintString("\rrunnableTasks: ");
    mprintString("\rblockedTasks:  ");
    mprintString("\rcurrentBuffer: ");

    while (1)
    {
        GoToSleep(20);

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
        msetcursor(7, 18);
        mprint64((long)currentBuffer);
        char c = mgetkey();
        if (c != -1)
        {
            msetcursor(10, 0);
            mprintchar(c);
        }
    }
}
