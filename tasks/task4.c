#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"

int main(void)
{
   WriteString("Task 4 is now running.", 14, 50);
   struct FCB *fHandle = OpenFile("TEST.TXT");
   WriteDouble((long)fHandle, 24, 40);
   if (fHandle)
   {
      DeleteFile(fHandle);
   }
   while (1)
   {
      WriteDouble(GetTicks(), 23, 60);
      sys_Sleep(100);
   }
   return(0);
}
