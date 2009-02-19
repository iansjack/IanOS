#include "cmemory.h"
#include "ckstructs.h"
#include "library/syscalls.h"

int main(void)
{
	WriteString("Task 3 is now running.", 13, 50);
	while (1)
	{
		WriteDouble(GetTicks(), 22, 60);
	}
	return 0;
}
