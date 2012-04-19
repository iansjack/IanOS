#include "syscalls.h"
#include "lib.h"
#include "console.h"
#include "filesystem.h"

int factorial(int n)
{
	if (n == 1) return 1;
	return factorial(n-1);
}

int main(int argc, char **argv)
{
	int x;

	x = factorial(100000);
//	printf("%d\n", factorial(500));
	exit();
	return (0);
}
