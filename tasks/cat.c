#include "kstructs.h"
#include "library/syscalls.h"
#include "library/lib.h"

int
main(int argc, char **argv)
{
	struct FCB * InFile = Sys_Open(argv[1]);
   if (InFile)
   {
      struct FileInfo inf;
      GetFileInfo(InFile, &inf);

      char * buffer = (char *) sys_AllocMem(inf.Length + 1);
      ReadFile(InFile, buffer, inf.Length);
      buffer[inf.Length] = 0;
      WriteConsoleString(buffer);
	   Sys_Close(InFile);
      sys_DeallocMem(buffer);
   }
   sys_Exit();
   return (0);
}
