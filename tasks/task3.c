#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"
#include "fat.h"

int main(void)
{
   struct DirEntry entry;
   long directory;

    sys_WriteString("Task 3 is now running.", 13, 50);
    writeconsolestring(sys_GetCommandLine(), 0);
    int count;
    for (count = 0; count < 10; count++)
    {
        sys_WriteDouble(sys_GetTicks(), 22, 60);
        sys_Sleep(50);
    }
    sys_WriteString("Task 3 is now ending. ", 13, 50);
    //sys_SetCurrentDirectory(8);
    GetDirectoryEntry(0, &entry);
    directory = GetDirectory("TESTDIR");
    if (directory == -1)
       writeconsolestring("Directory not found!");
    else
       writeconsolechar(directory + '0');
    sys_KillTask();
    return(0);
}
