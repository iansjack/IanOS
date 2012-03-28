/*
 * edit.c
 *
 *  Created on: Mar 26, 2012
 *      Author: ian
 */
#include "lib.h"
#include "syscalls.h"

#define ctrl(x) x - 0x40
#define ESC	27

struct line
{
	struct line *next;
	struct line *prev;
	int lineno;
	unsigned char *line;
};

struct line *lines;

void ReadFile(FD file)
{
	struct line *firstline = lines;
	struct line *currline;

	// First read the whole file into a buffer
	struct FileInfo info;
	fstat(file, &info);
	int filesize = info.Length;
	unsigned char *buffer = malloc(filesize);
	read(file, buffer, filesize);

	// Parse the buffer into separate lines
	lines = malloc(sizeof (struct line));
	lines->next = 0;
	lines->prev = 0;
	lines->lineno = 0;
	currline = lines;

	int i;
	while (filesize)
	{
		while (buffer[i] != 0x0a && i < filesize) i++;
		buffer[i] = 0;
		currline->line = malloc(i + 1);
		strcpy(currline->line, buffer);
		buffer = buffer + i + 1;
		filesize -= i + 1;
		i = 0;
		currline->next = malloc(sizeof (struct line));
		currline->next->prev = currline;
		currline = currline->next;
	}
}

int main(int argc, char **argv)
{
	lines = 0;

	FD file;
	int column = 0;
	int line = 0;
	struct line *currline;

	// Clear the screen
	printf("%c[2J", ESC);

	if (argc == 2)
	{
		file = open(argv[1]);
		if (file != -1)
		{
			ReadFile(file);
			currline = lines;
			while (currline)
			{
				if (currline->line)
					printf("%s\n", currline->line);
				currline = currline->next;
			}
			currline = lines;
			printf("%c[H", ESC);
		}
		else
		{
			file = creat(argv[1]);
			lines = malloc(sizeof (struct line));
			lines->next = 0;
			lines->prev = 0;
			lines->lineno = 0;
			lines->line = malloc(80);
		}
	}
	int done = 0;

	char c;
	while (!done)
	{
		c = getchar();
		if (c == ctrl('Q'))
			done = 1;
		else if (c == ctrl('M'))
		{
			c = 'J' - 0x40;
			printf("%c", c);
		}
		else if (c == ctrl('U'))
			printf("%c[1A", ESC);
		else if (c == ctrl('D'))
			printf("%c[1B", ESC);
		else if (c == ctrl('L'))
			printf("%c[1D", ESC);
		else if (c == ctrl('R'))
			printf("%c[1C", ESC);
		else
		{
			printf("%c", c);
			lines->line[column++] = c;
		}
	}
	lines->line[column++] = 0x0a;

	// Clear the screen
	printf("%c[2J", ESC);
	if (file != -1)
	{
		write(file, lines->line, column);
		close(file);
	}
	free(lines->line);
	free(lines);
	exit();
	return 0;
}
