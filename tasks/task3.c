#include "memory.h"
#include "kstructs.h"
#include "library/syscalls.h"

int main(void)
{
    sys_WriteString("Task 3 is now running.", 13, 50);
    writeconsolestring(sys_GetCommandLine(), 0);
    int count;
    for (count = 0; count < 10; count++)
    {
        sys_WriteDouble(sys_GetTicks(), 22, 60);
        sys_Sleep(50);
    }
    sys_WriteString("Task 3 is now ending. ", 13, 50);
    sys_KillTask();
    return(0);
}
