#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"

int main(void)
{
   sys_WriteString("Task 3 is now running.", 13, 50);
	writeconsolestring(sys_GetCommandLine());
   while (1)
   {
      sys_WriteDouble(sys_GetTicks(), 22, 60);
      sys_Sleep(50);
   }
   return(0);
}
