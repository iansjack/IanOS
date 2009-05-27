#include "cmemory.h"
#include "ckstructs.h"
#include "console.h"

//====================================================
// This is the task that listens for console requests.
//====================================================

short int      column;
short int      row;
char           *VideoBuffer;
struct Message ConsoleMsg;

void printchar(unsigned char c)
{
   switch (c)
   {
   case 0:
      break;

   case BACKSPACE:
      if (column > 0)
      {
         column--;
      }
      break;

   case CR:
      column = 0;
      row++;
      break;

   default:
      VideoBuffer[160 * row + 2 * column] = c;
      column++;
      if (column == 80)
      {
         column = 0;
         row++;
      }
   }
}


void consoleTaskCode()
{
   column      = 0;
   row         = 0;
   VideoBuffer = (char *)0xB8000;
   ((struct MessagePort *)ConsolePort)->waitingProc = (struct Task *)-1L;
   ((struct MessagePort *)ConsolePort)->msgQueue    = 0;

   unsigned char *s;

   while (1)
   {
      ReceiveMessage(ConsolePort, &ConsoleMsg);
      switch (ConsoleMsg.byte)
      {
      case WRITECHAR:
         printchar((unsigned char)ConsoleMsg.quad);
         break;

      case WRITESTR:
         s = (unsigned char *)ConsoleMsg.quad;
         while (*s != 0)
         {
            printchar(*s);
            s++;
         }
         DeallocMem((void *)ConsoleMsg.quad);
         break;

      case CLRSCR:
         for (row = 0; row < 25; row++)
         {
            for (column = 0; column < 80; column++)
            {
               VideoBuffer[160 * row + 2 * column]     = ' ';
               VideoBuffer[160 * row + 2 * column + 1] = 7;
            }
         }
         column = 0;
         row    = 0;
         break;

      default:
         break;
      }
   }
}
