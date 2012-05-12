#include <lib.h>

int main(int argc, char **argv)
{
	struct DirEntry entry;
	int ret;
	int count1, count2;
	unsigned char buffer[13];
	unsigned char *cwd;

	FD file;
	if (argc == 2)
		file = open(argv[1]);
	else
	{
		cwd = getcwd();
		file = open(cwd);
		free(cwd);
	}
	if (file != -1)
	{
		for (count1 = 0; count1 < 40; count1++)
		{
			ret = read(file, (char *) &entry, sizeof(struct DirEntry));
			if (entry.attribute != 0x08)	// Volume lable, not really a directory entry
			{
				if (entry.name[0])
				{
					if (entry.name[0] != 0xE5)
					{
						for (count2 = 0; count2 < 11; count2++)
							buffer[count2] = entry.name[count2];
						buffer[12] = 0;
						printf(buffer);
						if (entry.attribute & 0x10)
							printf(" <DIR> ");
						else
							printf("       ");
						buffer[8] = 0;
						intToAsc(entry.fileSize, buffer, 8);
						printf("%s ", buffer);
						int day = entry.modifiedDate & 0x001F;
						int month = (entry.modifiedDate & 0x01E0) >> 5;
						int year = ((entry.modifiedDate & 0xFE00) >>9) + 1980;
						int sec = 2 * (entry.modifiedTime & 0x001F);
						int min = (entry.modifiedTime & 0x07E0) >> 5;
						int hour = (entry.modifiedTime & 0xF800) >>11;
						printf("%2d-%02d-%4d %2d:%02d:%02d\n", day, month, year, hour, min, sec);
					}
				}
				else
					break;
			}
		}
		close(file);
	}
	exit();
	return (0);
}
