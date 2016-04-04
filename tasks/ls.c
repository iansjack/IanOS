#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ext2_fs.h"
#include <reent.h>

//static struct _reent reent;

int main(int argc, char **argv)
{
	char *buffer;
	char *cwd = malloc(256);

	int file;
	if (argc == 2)
	{
		chdir(argv[1]);
	}
	(void) getcwd(cwd, 256);
	file = open(cwd, O_RDONLY);
	free(cwd);
	if (file != -1)
	{
		struct stat inf;
		int dir_length;
		struct ext2_dir_entry_2 *dir;
		time_t t;

		fstat(file, &inf);
		dir_length = inf.st_size;
		buffer = malloc((size_t) dir_length);
		dir = (struct ext2_dir_entry_2 *) buffer;
		(void) read(file, buffer, (size_t) dir_length);
		while (1)
		{
			if (dir->inode)
			{
				char * c = malloc(dir->name_len + 16);
				char mode[] = "----------";
				struct tm *time;

				c[0] = 0;
				strncat(c, dir->name, dir->name_len);
				c[dir->name_len] = 0;
				lstat(c, &inf);
				if (inf.st_mode & S_IFDIR)
					mode[0] = 'd';
				if (inf.st_mode & S_IRUSR)
					mode[1] = 'r';
				if (inf.st_mode & S_IWUSR)
					mode[2] = 'w';
				if (inf.st_mode & S_IXUSR)
					mode[3] = 'x';
				if (inf.st_mode & S_IRGRP)
					mode[4] = 'r';
				if (inf.st_mode & S_IWGRP)
					mode[5] = 'w';
				if (inf.st_mode & S_IXGRP)
					mode[6] = 'x';
				if (inf.st_mode & S_IROTH)
					mode[7] = 'r';
				if (inf.st_mode & S_IWOTH)
					mode[8] = 'w';
				if (inf.st_mode & S_IXOTH)
					mode[9] = 'x';
				t = inf.st_mtime;
				time = gmtime(&t);
				printf("%s %4d %4d %02d:%02d %2d/%02d/%4d ", mode,
						(int) inf.st_uid, (int) inf.st_gid, time->tm_hour,
						time->tm_min, time->tm_mday, time->tm_mon + 1,
						1900 + time->tm_year);
				printf("%8d %s\n", inf.st_size, c);
				free(c);
			}
			dir = (struct ext2_dir_entry_2 *) ((char *) dir + dir->rec_len);
			if ((char *) dir - (char *) buffer >= dir_length)
				break;
		}
		free(buffer);
		close(file);
	}
	return (0);
}
