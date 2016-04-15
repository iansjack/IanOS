#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	ssize_t ret;

	int InFile = open(argv[1], O_RDONLY);
	if (InFile <= 0)
		printf("Error opening file %s\n", argv[1]);
	else
	{
		struct stat inf;
		char *buffer;

		fstat(InFile, &inf);

		buffer = (char *)malloc((size_t) inf.st_size + 1);
		ret = read(InFile, buffer, (size_t) inf.st_size);
		buffer[inf.st_size] = 0;
		printf("%s", buffer);
		close(InFile);
		free(buffer);
	}
	return (0);
}
