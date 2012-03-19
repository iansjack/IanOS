#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"
#include "fat.h"

int main(void)
{
	FD fileDescriptor = Sys_Creat("TEST.TXT");
	if (fileDescriptor != -1) {
		Sys_Write(fileDescriptor, "1234\n", 5);
		Sys_Close(fileDescriptor);
	}
	sys_Exit();
	return (0);
}
