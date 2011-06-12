#include "kstructs.h"
#include "library/syscalls.h"
#include "library/lib.h"

int main(void)
{
   char * buffer = (char *)sys_AllocMem(80);
   buffer = (char *)sys_GetCommandLine();
   buffer[13] = 0;

   struct FCB * InFile = OpenFile(&buffer[4]);
   // sys_WriteDouble((long)InFile, 23, 40);
   ReadFile(InFile, buffer, 4 /* InFile->length */);
   buffer[4 /*InFile->length*/] = '\r';
   buffer[5] = 0;
   WriteConsoleString(buffer);
   CloseFile(InFile);
   sys_DeallocMem(buffer);
   sys_KillTask();
   return(0);
}
