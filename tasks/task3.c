#include "syscalls.h"

int main(void)
{
	unlink("TEST.TXT");
	exit();
	return (0);
}
