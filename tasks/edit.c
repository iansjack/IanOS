/*
 * edit.c
 *
 *  Created on: Mar 26, 2012
 *      Author: ian
 */
#include "lib.h"
#include "syscalls.h"

#define ESC			27

int main(int argv, char **argc)
{
	int done = 0;

	// Clear the screen
	printf("%c[2J", ESC);

	char c = ' ';
	while (!done)
	{
		c = getchar();
		switch (c)
		{
		case 'Q' - 0x40:
			done = 1;
			break;

		case 'M' - 0x40:
			c = 'J' - 0x40;

		default:
			printf("%c", c);
			break;
		}
	}
	// Clear the screen
	printf("%c[2J", ESC);
	exit();
	return 0;
}
