#include "kstructs.h"
#include "library/syscalls.h"

int main(void)
{
   sys_WriteString("Hi", 22, 40);
	long pid = GetFSPID();
	sys_WriteDouble(pid, 23, 20);
   struct FCB *fHandle = CreateFile("TEST.TXT");
   sys_WriteDouble((long)fHandle, 23, 40);
   if (fHandle)
   {
      WriteFile(fHandle, "1234\n", 5);
      CloseFile(fHandle);
   }
   while (1)
   {
      sys_Sleep(200);
   }
   return(0);
}
