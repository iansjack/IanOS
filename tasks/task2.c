#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"
#include "fat.h"

int main(void)
{
	unsigned char fileDescriptor = Sys_Creat("TEST.TXT");
	if (fileDescriptor)
	{
		Sys_Write(fileDescriptor, "1234\n", 5);
		Sys_Close(fileDescriptor);
	}
	sys_Exit();
	return (0);
}
