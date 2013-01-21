#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int ret;

	int InFile = open(argv[1]);
	if (InFile == -1)
		printf("Error opening file %s\n", argv[1]);
	else
	{
		struct stat inf;
		fstat(InFile, &inf);

		char *buffer = (char *)malloc(inf.st_size + 1);
		ret = read(InFile, buffer, inf.st_size);
		buffer[inf.st_size] = 0;
		printf("%s", buffer);
		close(InFile);
		free(buffer);
	}
	return (0);
}
