#include "kstructs.h"
#include "memory.h"
#include "syscalls.h"
#include "lib.h"
#include "console.h"

int
main(void)
{
   struct Message m;
   int column = 0;
   char commandline[81];
   char buffer[512];
   char name[81];
   char environment[81];
	char s[2];
   int i;

   ConsoleClrScr();
	printf("IanOS Version 0.1.3 - 2012\n#> _\b");
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
            printf(" \b\b_\b");
            commandline[column--] = 0;
         }
         break;

      case CR:
         commandline[column] = 0;
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

         printf(" \n");

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
               printf("Directory not found!");
            else
               sys_SetCurrentDirectory(dir);
         }
         else
         {
			int pid = Sys_Fork();
			if (!pid)
				Sys_Execve(name, environment);
			 else
				 Sys_Wait(pid);
         }

         printf("#> _\b");

         // Clear commandline[]
         for (i = 0; i < 80; i++)
         {
            commandline[i] = ' ';
         }
         break;

      default:
		commandline[column++] = c;
		s[0] = c;
		s[1] = 0;
		printf(s);
        printf("_\b");
        break;
        }
   }
   return (0);
}
