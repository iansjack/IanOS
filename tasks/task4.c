#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv)
{
	long t;
	printf("%d\n", sys_time());
	time(&t);
	printf("%d\n", t);
}
