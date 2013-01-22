#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char *cwd = malloc(256);
	cwd = getcwd(cwd, 256);
	printf("%s\n", cwd);
	free(cwd);
	return (0);
}
