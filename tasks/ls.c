#include "kstructs.h"
#include "library/syscalls.h"
#include "fat.h"

int main(void)
{
   struct DirEntry entry;
   int count1, count2;
   char buffer[9];

   buffer[8] = 0;
   for (count1 = 0; count1 < 200; count1++)
   {
      GetDirectoryEntry(count1, &entry);
      if (entry.name[0] == 0) break;
      if (entry.name[0] != 0xE5)
      {
         for (count2 = 0; count2 < 11; count2++) WriteConsoleChar(entry.name[count2]);
         if (entry.attribute & 0x10)
            WriteConsoleString(" <DIR> ");
         else
            WriteConsoleString("       ");
         intToAsc(entry.fileSize, buffer, 8);
         WriteConsoleString(buffer);
         WriteConsoleChar('\r');
      }
   }
   sys_KillTask();
   return(0);
}
