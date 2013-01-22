//#include "syscalls.h"

int main(int argc, char **argv)
{
	if (argc == 2)
		unlink(argv[1]);
	exit();
	return (0);
}
