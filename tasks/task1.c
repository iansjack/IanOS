#include "ckstructs.h"
#include "cmemory.h"
#include "library/syscalls.h"
#include "library/lib.h"
#include "console.h"

int main(void)
{
	struct Message *taskMsg;
	int column = 0;
	char commandline[80];
	char buffer[512];
	char * name = sys_AllocSharedMem(80);

	consoleclrscr();
	writeconsolestring("IanOS Version 0.1 - 2008\r");
	writeconsolestring("#> _\b");
	sys_CreateTask("TASK2.BIN");
	sys_Sleep(10);
    
	while (1)
	{
		char c = getchar();
	    
		switch (c)
		{
			case BACKSPACE:
				if (column > 0)
				{	
					writeconsolestring(" \b\b_\b");
					commandline[column--] = 0;
				}
				break;
			case CR:
				column = 0;
				writeconsolestring(" \r#> _\b");

				// Convert commandline[] to uppercase.
				char i;
				for (i = 0; i < 12; i++)
				{
					if (commandline[i] >= 'a' & commandline[i] <= 'z') 
						commandline[i] = commandline[i] - 0x20;
				}
				for (i = 0; i < 80; i++) name[i] = commandline[i];
				sys_CreateTask(name);

				// Clear commandline[]
				for (i = 0; i < 12; i++) commandline[i] = 0;
				break;
			default:
				commandline[column++] = c;
				writeconsolechar(c);
				writeconsolestring("_\b");
				break;
		}
	}
	return 0;
}

