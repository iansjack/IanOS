#include "kstructs.h"
#include "memory.h"
#include "syscalls.h"
#include "lib.h"
#include "console.h"

int main(int argc, char **argv)
{
	int column = 0;
	char commandline[81];
	char buffer[512];
	char name[81];
	char environment[81];
	int i;

	if (argc == 2)
		if (argv[1][0] >= '0' && argv[1][0] <= '3') {
			i = argv[1][0] - '0';
			printf("%c#%d", ESC, i);
		}
	printf("%c[2J", ESC);
	printf("IanOS Version 0.1.3 - 2012  Console %d\n#> ", i + 1);
	name[80] = environment[80] = 0;
	for (i = 0; i < 80; i++)
		commandline[i] = ' ';

	while (1) {
		char c = getchar();

		switch (c) {
		case BACKSPACE:
			if (column > 0) {
				printf("\b \b");
				commandline[column--] = 0;
			}
			break;

		case CR:
			commandline[column] = 0;
			column = 0;

			i = 0;
			while (commandline[i] != ' ') {
				name[i] = commandline[i];
				i++;
			}
			name[i] = 0;

			// Convert name[] to upper case.
			i = -1;
			while (name[++i]) {
				if (name[i] >= 'a' & name[i] <= 'z')
					name[i] = name[i] - 0x20;
			}

			for (i = 0; i < 80; i++)
				environment[i] = commandline[i];

			printf(" \n");

			// Process "CD" command;
			if (name[0] == 'C' && name[1] == 'D' && name[2] == 0) {
				char directory[80];
				for (i = 0; i < 77; i++)
					if (environment[i + 3] != ' ')
						directory[i] =
						    environment[i + 3];
					else
						break;
				directory[i] = 0;
				int dir = chdir(directory);
				if (dir == -1)
					printf("Directory not found!\n");
			} else {
				int pid = fork();
				if (!pid)
				{
					if (execve(name, environment))
					{
						printf("Command not found\n");
						exit();
					}
				}
				else
					wait(pid);
			}

			printf("#> ");

			// Clear commandline[]
			for (i = 0; i < 80; i++) {
				commandline[i] = ' ';
			}
			break;

		default:
			commandline[column++] = c;
			printf("%c", c);
			break;
		}
	}
	return (0);
}
