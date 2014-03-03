#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "console.h"
#undef errno
extern int errno;

int main(int argc, char **argv)
{
	int column = 0;
	char commandline[81];
	char environment[81];

	printf("%c[2J", ESC);
	printf("IanOS Version 0.2.0 - 2014  Improved Shell\n#>");
	environment[80] = 0;
	memset(commandline, ' ', 80);
	while (1)
	{
		char *name, c;

		fflush(stdout);
		c = getchar();
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
			name = strtok(commandline, " ");
			printf("\n");

			if (name)
			{
				// Process "cd" command;
				if (!strcmp(name, "cd"))
				{
					char *directory = strtok(/*NULL*/ 0, " ");
					if (chdir(directory) == -1)
						printf("Directory not found!\n");
				}

				// Process "exit" command
				else if (!strcmp(name, "exit"))
					return 0;

				else
				{
					pid_t pid = fork();
					if (!pid)
					{
						if (execve(name, (char **)environment, NULL))
						{
							if (errno == ENOENT)
								printf("Command not found\n");
							else
								printf("Not an executable: errorno = %d\n", errno);
							return 1;
						}
					}
					else
						(void) waitpid(pid, NULL, 0);
				}
			}

			printf("#>");

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
