#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
	ssize_t ret;
	int InFile, OutFile;

	InFile = open(argv[1], O_RDONLY);
	OutFile = open(argv[2], O_WRONLY | O_CREAT);
	if (InFile == -1)
		printf("Error opening file %s\n", argv[1]);
	else
	{
		struct stat inf;
		char *buffer;

		fstat(InFile, &inf);

		buffer = (char *)malloc((size_t) inf.st_size);
		ret = read(InFile, buffer, (size_t) inf.st_size);
		write(OutFile, buffer, (size_t) inf.st_size);
		close(InFile);
		close(OutFile);
		free(buffer);
	}
	return (0);
}
