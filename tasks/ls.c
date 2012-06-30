#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ext2_fs.h"

int main(int argc, char **argv)
{
	char *buffer;
	char *cwd = malloc(256);

	int file;
	if (argc == 2)
		file = open(argv[1]);
	else
	{
		getcwd(cwd, 256);
		file = open(cwd);
		free(cwd);
	}
	if (file != -1)
	{
		struct stat inf;
		fstat(file, &inf);
		int dir_length = inf.st_size;
		buffer = malloc(dir_length);
		struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *) buffer;
		read(file, buffer, dir_length);
		while (1)
		{
			char * c = malloc(dir->name_len + 16);
			strncpy(c, dir->name, dir->name_len);
			c[dir->name_len] = 0;
			lstat(c, &inf);
			time_t t = inf.st_atime;
			struct tm *time = gmtime(&t);
			printf("%2d:%2d %2d/%2d/%4d ", time->tm_hour, time->tm_min, time->tm_mday, time->tm_mon + 1, 1900 + time->tm_year);
			printf("%8d %s\n", inf.st_size, c);
			dir = (struct ext2_dir_entry_2 *) ((char *) dir + dir->rec_len);
			if ((char *) dir - (char *) buffer >= dir_length)
				break;
			free(c);
		}
		free(buffer);
		close(file);
	}
	return (0);
}
