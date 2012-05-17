#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "console.h"

int main(int argc, char **argv)
{
	int column = 0;
	char commandline[81];
	char environment[81];

	printf("%c[2J", ESC);
	printf("IanOS Version 0.1.4 - 2012  Improved Shell\n#>");
	environment[80] = 0;
	memset(commandline, ' ', 80);
	while (1)
	{
		char c = getchar();

		switch (c)
		{
		case BACKSPACE:
			if (column > 0)
			{
				printf("\b \b");
				commandline[column--] = 0;
			}
			break;

		case CR:
			commandline[column] = 0;
			column = 0;

			memcpy(environment, commandline, 80);
			char *name = strtok(commandline, " ");

			// Convert name[] to upper case.
			strupr(name);
			printf("\n");

			// Process "CD" command;
			if (!strcmp(name, "CD"))
			{
				char *directory = strtok(NULL, " ");
				if (chdir(directory) == -1)
					printf("Directory not found!\n");
			}

			// Process "EXIT" command
			else if (!strcmp(name, "EXIT"))
				return 0;
			else
			{
				int pid = fork();
				if (!pid)
				{
					if (execve(name, environment))
					{
						printf("Command not found\n");
						return 1;
					}
				}
				else
					waitpid(pid);
			}

			printf("#> ");

			// Clear commandline[]
			memset(commandline, ' ', 80);
			break;

		case 4: // Down arrow
		case 12: // Left arrow
		case 18: // Right arrow
		case 21: // Up arrow
			break;

		default:
			commandline[column++] = c;
			printf("%c", c);
			break;
		}
	}
	return (0);
}
