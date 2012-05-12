#include "kstructs.h"
#include "syscalls.h"
#include "lib.h"
#include "fat.h"

int main(void)
{
	FD fileDescriptor = creat("TEST.TXT");
	if (fileDescriptor != -1) {
		write(fileDescriptor, "1234\n", 5);
		close(fileDescriptor);
	}
	exit();
	return (0);
}
