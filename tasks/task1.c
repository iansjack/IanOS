#include "kstructs.h"
#include "memory.h"
#include "library/syscalls.h"
#include "library/lib.h"
#include "console.h"

int
main(void)
{
   struct Message m;
   int column = 0;
   char commandline[81];
   char buffer[512];
   char *name = sys_AllocSharedMem(81);
   char *environment = sys_AllocSharedMem(81);
   int i;

   consoleclrscr();
   writeconsolestring("IanOS Version 0.1.2 - 2011\r");
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

         i = 0;
         while (commandline[i] != ' ')
         {
            name[i] = commandline[i];
            i++;
         }
         name[i] = 0;

         // Convert name[] to upper case.
         i = -1;
         while (name[++i])
         {
            if (name[i] >= 'a' & name[i] <= 'z')
               name[i] = name[i] - 0x20;
         }

         for (i = 0; i < 80; i++)
         {
            environment[i] = commandline[i];
         }

         writeconsolestring(" \r");

         // Process "CD" command;
         if (name[0] == 'C' && name[1] == 'D' && name[2] == 0)
         {
            char directory[80];
            for (i = 0; i < 77; i++)
               if (environment[i + 3] != ' ')
                  directory[i] = environment[i + 3];
               else
                  break;
            directory[i] = 0;
            int dir = GetDirectory(directory);
            if (dir == -1)
               writeconsolestring("Directory not found!");
            else
            {
               writeconsolechar(dir + '0');
               sys_SetCurrentDirectory(dir);
            }
         }
         else
         {
            if (name[0] == '&')
            {
               sys_CreateTask(name + 1, environment + 1, 0,
                     sys_GetCurrentConsole());
            }
            else
            {
               struct Message *msg = (struct Message *) sys_AllocMem(
                     sizeof(struct Message));
               struct MessagePort * parentPort = sys_AllocMessagePort();
               sys_CreateTask(name, environment, parentPort,
                     sys_GetCurrentConsole());
               sys_ReceiveMessage((long int) parentPort, msg);
               sys_DeallocMem(parentPort);
               sys_DeallocMem(msg);
            }
         }

         writeconsolestring("#> _\b");

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
   return (0);
}
