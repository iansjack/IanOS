#include "cmemory.h"
#include "ckstructs.h"

#define KBDINT      1

#define LSHIFT      42
#define RSHIFT      54
#define ALT         56
#define CAPSLOCK    58

#define SHIFTED     1
#define LOCKED      2
#define CTRLED      4
#define ALTED       8

long          kbBufStart;
long          kbBufCurrent;
unsigned char kbBufCount;
unsigned char kbBuffer[128];
unsigned char modifier;

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

//=====================================================
// This is the task that listens for keyboard requests.
//=====================================================
void kbTaskCode()
{
   unsigned char      temp;
   struct MessagePort *tempPort;
   struct Message     *KbdMsg;

   kbBufStart = kbBufCurrent = kbBufCount = modifier = 0;

   asm ("mov $0b11111000, %al");        // enable keyboard + timer interrupt"
   asm ("out %al, $0x21");

   KbdMsg = (struct Message *)AllocKMem(sizeof(struct Message));

   ((struct MessagePort *)KbdPort)->waitingProc = (struct Task *)-1L;
   ((struct MessagePort *)KbdPort)->msgQueue    = 0;
   while (1)
   {
      ReceiveMessage(KbdPort, KbdMsg);
      if (KbdMsg->byte == 1)
      {
         temp = 0x80;
         while (temp >= 0x80)
         {
            if (kbBufCount == 0)
            {
               WaitForInt(KBDINT);
            }
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

               default:
                  if (modifier & SHIFTED)
                  {
                     temp = KbdTableS[temp];
                  }
                  else
                  {
                     temp = KbdTableU[temp];
                  }
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
            kbBufCount--;
            kbBufStart++;
            if (kbBufStart == 128)
            {
               kbBufStart = 0;
            }
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

         tempPort            = (struct MessagePort *)KbdMsg->tempPort;
         KbdMsg->nextMessage = 0;
         KbdMsg->quad        = 0L;
         KbdMsg->byte        = temp;
         SendMessage(tempPort, KbdMsg);
      }
      else
      {
      }
   }
}
