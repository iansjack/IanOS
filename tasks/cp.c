#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

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
		int toCopy;

		fstat(InFile, &inf);
		toCopy = inf.st_size;

		buffer = (char *)malloc((size_t) BUFF_SIZE);

		while (toCopy > BUFF_SIZE)
		{
			read(InFile, buffer, (size_t) BUFF_SIZE);
			write(OutFile, buffer, (size_t) BUFF_SIZE);
			toCopy -= BUFF_SIZE;
		}
		read(InFile, buffer, (size_t) toCopy);
		write(OutFile, buffer, (size_t) toCopy);

		close(InFile);
		close(OutFile);
		free(buffer);
	}
	return (0);
}
