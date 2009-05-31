#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"

int main(void)
{
   WriteString("Task 3 is now running.", 13, 50);
	writeconsolestring(sys_GetCommandLine());
   while (1)
   {
      WriteDouble(GetTicks(), 22, 60);
      sys_Sleep(50);
   }
   return(0);
}
