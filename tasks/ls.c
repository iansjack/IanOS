#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"
#include "fat.h"

int main(void)
{
   struct DirEntry entry;
   int count1, count2;
   char buffer[12];

   buffer[8] = 0;
   for (count1 = 0; count1 < 200; count1++)
   {
      GetDirectoryEntry(count1, &entry);
      if (entry.name[0] == 0) break;
      if (entry.name[0] != 0xE5)
      {
         for (count2 = 0; count2 < 11; count2++) 
			  buffer[count2] = entry.name[count2];
		  buffer[12] = 0;
		  printf(buffer);
         if (entry.attribute & 0x10)
            printf(" <DIR> ");
         else
            printf("       ");
		  buffer[8] = 0;
         intToAsc(entry.fileSize, buffer, 8);
         printf(buffer);
         printf("\n");
      }
   }
   sys_Exit();
   return(0);
}
