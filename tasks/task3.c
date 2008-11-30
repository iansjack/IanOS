#include "cmemory.h"
#include "ckstructs.h"
#include "library/syscalls.h"

void main(void)
{
	WriteString("Task 3 is now running.", 13, 50);
	//writeconsolechar('Y');
	while (1)
	{
		WriteDouble(GetTicks(), 22, 60);
	}
}
