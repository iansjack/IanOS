#include "syscalls.h"

int main(void)
{
	Sys_Unlink("TEST.TXT");
    sys_Exit();
    return(0);
}
