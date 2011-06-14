#include "kstructs.h"
#include "library/syscalls.h"
#include "library/lib.h"

int
main(int argc, char **argv)
{
   char * buffer = (char *) sys_AllocMem(80);

   struct FCB * InFile = OpenFile(argv[1]);
   ReadFile(InFile, buffer, 4 /* InFile->length */);
   buffer[4 /*InFile->length*/] = '\r';
   buffer[5] = 0;
   WriteConsoleString(buffer);
   CloseFile(InFile);
   sys_DeallocMem(buffer);
   sys_KillTask();
   return (0);
}
