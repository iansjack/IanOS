#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"
#include "library/lib.h"

int main(void)
{
   sys_WriteString("Task 4 is now running.", 14, 50);
   struct FCB *fHandle = OpenFile("TEST.TXT");
   sys_WriteDouble((long)fHandle, 24, 40);
   if (fHandle)
   {
      DeleteFile(fHandle);
   }
   while (1)
   {
      sys_WriteDouble(sys_GetTicks(), 23, 60);
      sys_Sleep(100);
   }
   return(0);
}
