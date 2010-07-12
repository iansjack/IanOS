#include "memory.h"
#include "kernel.h"

#define KBDINT       1

#define GETCHAR      1
#define KEYPRESS     2
#define GETKEY       3

#define LSHIFT       42
#define RSHIFT       54
#define ALT          56
#define CAPSLOCK     58
#define F1           59
#define F2           60
#define F3           61
#define F4           62
#define F5           63
#define F6           64
#define F7           65
#define F8           66

#define SHIFTED     1
#define LOCKED      2
#define CTRLED      4
#define ALTED       8

extern void switchConsole(long console);

long           kbBufStart;
long           kbBufCurrent;
unsigned char  kbBufCount;
unsigned char  kbBuffer[128];
unsigned char  modifier;
unsigned char  currentBuffer;
struct Console consoles[8];

//==============================
// The unshifted keyboard table.
//==============================

char KbdTableU[] =
{
    0,               0 /*esc*/, '1',           '2',              '3',            '4',          '5',       '6',            '7', '8', '9', '0',  '-', '=',          8 /*backspace*/,
    0 /*tab*/,    'q',          'w',           'e',              'r',            't',          'y',       'u',            'i', 'o', 'p', '[',  ']',           13,
    0 /*ctrl*/,   'a',          's',           'd',              'f',            'g',          'h',       'j',            'k', 'l', ';', '\'', '`', 0 /*lshift*/, '#',
    'z',          'x',          'c',           'v',              'b',            'n',          'm',       ',',            '.', '/',
    0 /*rshift*/, 0 /*sysreq*/,     0 /*alt*/, ' ',              0 /*capslock*/,     0 /*F1*/,  0 /*F2*/,       0 /*F3*/,
    0 /*F4*/,         0 /*F5*/,      0 /*F6*/,         0 /*F7*/,       0 /*F8*/,
    0 /*F9*/,        0 /*F10*/, 0 /*numlock*/, 0 /*scrolllock*/,     0 /*home*/, 0 /*uarrow*/, 0 /*pup*/, 0 /*numminus*/,
    0 /*larrow*/,   0 /*num5*/,  0 /*rarrow*/,    0 /*numplus*/,      0 /*end*/,
    0 /*darrow*/,  0 /*pdown*/,     0 /*ins*/,        0 /*del*/,              0,            0, '\\',           0 /*F11*/, 0 /*F12*/
};

//==============================
// The shifted keyboard table.
//==============================

char KbdTableS[] =
{
    0,               0 /*esc*/, '!',           '"',              '#',            '$',          '%',       '^',              '&', '*', '(', ')', '_', '+',          8 /*backspace*/,
    0 /*tab*/,    'Q',          'W',           'E',              'R',            'T',          'Y',       'U',              'I', 'O', 'P', '{', '}',           13,
    0 /*ctrl*/,   'A',          'S',           'D',              'F',            'G',          'H',       'J',              'K', 'L', ':', '@', '`', 0 /*lshift*/, '#',
    'Z',          'X',          'C',           'V',              'B',            'N',          'M',       '<',              '>', '?',
    0 /*rshift*/, 0 /*sysreq*/,     0 /*alt*/, ' ',              0 /*capslock*/,     0 /*F1*/,  0 /*F2*/,         0 /*F3*/,
    0 /*F4*/,         0 /*F5*/,      0 /*F6*/,         0 /*F7*/,       0 /*F8*/,
    0 /*F9*/,        0 /*F10*/, 0 /*numlock*/, 0 /*scrolllock*/,     0 /*home*/, 0 /*uarrow*/, 0 /*pup*/, '-' /*numminus*/,
    0 /*larrow*/, '5' /*num5*/,  0 /*rarrow*/, '+' /*numplus*/,       0 /*end*/,
    0 /*darrow*/,  0 /*pdown*/,     0 /*ins*/,        0 /*del*/,              0,            0, '|',              0 /*F11*/, 0 /*F12*/
};

//==========================================================
// Switch the variables to point to the current console
//==========================================================
void setBuffer()
{
    struct Message *kbdMsg = (struct Message *)AllocKMem(sizeof(struct Message));

    kbdMsg->nextMessage = 0;
    kbdMsg->quad        = currentBuffer;
    kbdMsg->byte        = KEYPRESS;
    SendMessage((struct MessagePort *)KbdPort, kbdMsg);
    DeallocMem(kbdMsg);
}

//=====================================================
// This is the task that listens for keyboard requests.
//=====================================================
void kbTaskCode()
{
    unsigned char      temp;
    struct MessagePort *tempPort;
    struct Message     *KbdMsg;
    struct Console     *currentCons;
    struct Message     * tempMsg;


    int i;
    for (i = 0; i < 8; i++)
    {
        consoles[i].kbBuffer = AllocKMem(128);
        consoles[i].kbBufStart = consoles[i].kbBufCurrent = consoles[i].kbBufCount = 0;
        consoles[i].MsgQueue = 0;
    }

    kbBufStart = 0;
    kbBufCurrent = 0;
    kbBufCount = 0;
    currentBuffer = 0;

    KbdMsg = (struct Message *)AllocKMem(sizeof(struct Message));
    modifier = 0;

    asm ("mov $0b11111000, %al");        // enable keyboard + timer interrupt"
    asm ("out %al, $0x21");

    ((struct MessagePort *)KbdPort)->waitingProc = (struct Task *)-1L;
    ((struct MessagePort *)KbdPort)->msgQueue    = 0;
    while (1)
    {
        ReceiveMessage((struct MessagePort *)KbdPort, KbdMsg);
        switch (KbdMsg->byte)
        {
        case GETKEY:
            if (!consoles[currentBuffer].kbBufCount)
            {
                tempPort            = (struct MessagePort *)KbdMsg->tempPort;
                KbdMsg->nextMessage = 0;
                KbdMsg->quad        = 0L;
                KbdMsg->byte        = -1;
                SendMessage(tempPort, KbdMsg);
            }

        case GETCHAR:
            tempMsg = consoles[KbdMsg->quad].MsgQueue;
            if (!tempMsg) consoles[KbdMsg->quad].MsgQueue = KbdMsg;
            else
            {
                while (tempMsg->nextMessage)
                    tempMsg = tempMsg->nextMessage;
                tempMsg->nextMessage = KbdMsg;
            }
            KbdMsg = (struct Message *)AllocKMem(sizeof(struct Message));
            break;

        case KEYPRESS:
            currentCons = &(consoles[currentBuffer]);
            temp = kbBuffer[kbBufStart];
            if (temp < 0x80)
            {
                switch (temp)
                {
                case LSHIFT:
                case RSHIFT:
                    modifier += SHIFTED;
                    temp      = 0x80;
                    break;

                case CAPSLOCK:
                    if (modifier & LOCKED)
                    {
                        modifier -= LOCKED;
                    }
                    else
                    {
                        modifier += LOCKED;
                    }
                    temp = 0x80;
                    break;

                case F1:
                    switchConsole(0);
                    break;

                case F2:
                    switchConsole(1);
                    break;

                case F3:
                    switchConsole(2);
                    break;

                case F4:
                    switchConsole(3);
                    break;

                case F5:
                    switchConsole(4);
                    break;

                case F6:
                    switchConsole(5);
                    break;

                case F7:
                    switchConsole(6);
                    break;

                case F8:
                    switchConsole(7);
                    break;

                default:
                    if (modifier & SHIFTED)
                    {
                        temp = KbdTableS[temp];
                    }
                    else
                    {
                        temp = KbdTableU[temp];
                    }
                    currentCons->kbBuffer[currentCons->kbBufCurrent] = temp;
                    currentCons->kbBufCount++;
                    currentCons->kbBufCurrent++;
                }
            }
            else
            {
                switch (temp & 0x7F)
                {
                case LSHIFT:
                case RSHIFT:
                    modifier -= SHIFTED;
                    break;

                default:
                    break;
                }
            }
            (kbBufCount)--;
            (kbBufStart)++;
            if ((kbBufStart) == 128)
            {
                (kbBufStart) = 0;
            }

            if (modifier & LOCKED)
            {
                if (modifier & SHIFTED)
                {
                    if ((temp >= 'A') && (temp <= 'Z'))
                    {
                        temp += 0x20;
                    }
                }
                else
                {
                    if ((temp >= 'a') && (temp <= 'z'))
                    {
                        temp -= 0x20;
                    }
                }
            }

            while (currentCons->kbBufCount && currentCons->MsgQueue)
            {
                temp = currentCons->kbBuffer[currentCons->kbBufStart];
                currentCons->kbBufCount--;
                currentCons->kbBufStart++;
                tempMsg = currentCons->MsgQueue;
                tempPort            = (struct MessagePort *)tempMsg->tempPort;
                tempMsg->nextMessage = 0;
                tempMsg->quad        = 0L;
                tempMsg->byte        = temp;
                SendMessage(tempPort, tempMsg);
                currentCons->MsgQueue = tempMsg->nextMessage;
                DeallocMem(tempMsg);
            }
            break;

        default:
            break;
        }
    }
}
