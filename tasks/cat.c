#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"

int
main(int argc, char **argv)
{
	/*struct FCB * */ unsigned char InFile = Sys_Open(argv[1]);
   if (InFile)
   {
      struct FileInfo inf;
      Sys_Stat(InFile, &inf);

      char * buffer = (char *) sys_AllocMem(inf.Length + 1);
	   Sys_Read(InFile, buffer, inf.Length);
      buffer[inf.Length] = 0;
      WriteConsoleString(buffer);
	   Sys_Close(InFile);  // The problem is here, but only after doing a Sys_Stat!!!
      sys_DeallocMem(buffer);
   }
   sys_Exit();
   return (0);
}
