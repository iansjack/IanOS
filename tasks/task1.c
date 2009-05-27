#include "ckstructs.h"
#include "cmemory.h"
#include "library/syscalls.h"
#include "library/lib.h"
#include "console.h"

int main(void)
{
   struct Message *taskMsg;
   int            column = 0;
   char           commandline[81];
   char           buffer[512];
   char           *name        = sys_AllocSharedMem(81);
   char           *environment = sys_AllocSharedMem(81);
   int            i;

   consoleclrscr();
   writeconsolestring("IanOS Version 0.1 - 2008\r");
   writeconsolestring("#> _\b");
   name[80] = environment[80] = 0;
   for (i = 0; i < 80; i++)
   {
      commandline[i] = ' ';
   }

   while (1)
   {
      char c = getchar();

      switch (c)
      {
      case BACKSPACE:
         if (column > 0)
         {
            writeconsolestring(" \b\b_\b");
            commandline[column--] = 0;
         }
         break;

      case CR:
         column = 0;
         writeconsolestring(" \r#> _\b");

         // Convert commandline[] to uppercase.
         //for (i = 0; i < 12; i++)
         //{
         //	if (commandline[i] >= 'a' & commandline[i] <= 'z')
         //		commandline[i] = commandline[i] - 0x20;
         //}
         i = 0;
         while (commandline[i] != ' ')
         {
            name[i] = commandline[i];
            i++;
         }
         name[i] = 0;

         for (i = 0; i < 80; i++)
         {
            environment[i] = commandline[i];
         }
         sys_CreateTask(name, environment);

         // Clear commandline[]
         for (i = 0; i < 80; i++)
         {
            commandline[i] = ' ';
         }
         break;

      default:
         commandline[column++] = c;
         writeconsolechar(c);
         writeconsolestring("_\b");
         break;
      }
   }
   return(0);
}
