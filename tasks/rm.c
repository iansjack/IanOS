#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	unlink(argv[1]);
	return (0);
}
