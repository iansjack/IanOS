/*
 * edit.c
 *
 *  Created on: Mar 26, 2012
 *      Author: ian
 */
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define ctrl(x) x - 0x40
#define ESC	27
#define CLRBUFF	for (count = 0; count < 80; count++) currentLineBuffer[count] = 0
#define Overwrite	0
#define Insert		1
#define WINDOWSIZE	20

struct line
{
	struct line *next;
	struct line *prev;
	unsigned char *line;
	int lineno;
};

struct line *lines;
struct line *windowStart;
int line;
int column;
char mode;
int count;

void ReadFile(int file)
{
	struct line *firstline = lines;
	struct line *currline;

	// First read the whole file into a buffer
	struct stat info;
	fstat(file, &info);
	int filesize = info.st_size;
	unsigned char *buffer = malloc(filesize);
	read(file, buffer, filesize);

	// Parse the buffer into separate lines
	lines = malloc(sizeof(struct line));
	lines->next = 0;
	lines->prev = 0;
	lines->lineno = 0;
	currline = lines;

	char *line = strtok(buffer, "\n");
	while (line)
	{
		currline->line = malloc(strlen(line));
		strcpy(currline->line, line);
		line = strtok(NULL, "\n");
		if (line)
		{
			currline->next = malloc(sizeof(struct line));
			currline->next->prev = currline;
			currline = currline->next;
		}
	}
	free(buffer);
}

void PrintStatusLine()
{
	if (mode == Insert)
	{
		printf(
				"%c[24;1H%c[?5h^Q Quit  ^I Insert toggle  Insert on                                          ",
				ESC, ESC);
		printf("%c[%d;%dH%c[?5l", ESC, line, column, ESC);
	}
	else
	{
		// Print status line
		printf(
				"%c[24;1H%c[?5h^Q Quit  ^I Insert toggle  Insert off                                          ",
				ESC, ESC);
		printf("%c[%d;%dH%c[?5l", ESC, line, column, ESC);

	}
}

void RedrawScreen()
{
	struct line *tempcurrline = lines;
	printf("%c[2J", ESC);
	tempcurrline = windowStart;
	if (!windowStart)
		return;
	for (count = 0; count <= WINDOWSIZE; count++)
	{
		if (tempcurrline->line)
			printf("%s\n", tempcurrline->line);
		if (!tempcurrline->next)
			break;
		tempcurrline = tempcurrline->next;
	}
	PrintStatusLine();
}

int main(int argc, char **argv)
{
	lines = 0;

	int file;
	line = 0;
	column = 0;
	struct line *currline;
	char currentLineBuffer[80];
	int count;
	mode = Insert;

	// Clear the screen
	printf("%c[2J", ESC);

	if (argc == 2)
	{
		file = open(argv[1]);
		if (file != -1)
		{
			ReadFile(file);
			windowStart = lines;
			RedrawScreen();
		}
		else
		{
			lines = malloc(sizeof(struct line));
			lines->next = 0;
			lines->prev = 0;
			lines->lineno = 0;
			lines->line = malloc(80);
		}
		currline = lines;
	}
	int done = 0;
	PrintStatusLine();
	CLRBUFF;
	char c;
	if (currline->line)
		strcpy(currentLineBuffer, currline->line);

	while (!done)
	{
		c = getchar();
		switch (c)
		{
		case ctrl('Q'):
			done = 1;
			break;
		case ctrl('I'):
			if (mode == Insert)
				mode = Overwrite;
			else
				mode = Insert;
			PrintStatusLine();
			break;
		case ctrl('M'): // Return
			;
			int linelength = strlen(currentLineBuffer);
			currentLineBuffer[column] = 0;
			// Copy the line up to cursor back to currline->line
			free(currline->line);
			currline->line = malloc(strlen(currentLineBuffer) + 1);
			currentLineBuffer[column] = 0;
			strcpy(currline->line, currentLineBuffer);
			// Create a new struct line for the new line
			struct line *temp = malloc(sizeof(struct line));
			// Link it into the list
			temp->next = currline->next;
			currline->next = temp;
			if (temp->next)
				temp->next->prev = temp;
			temp->prev = currline;
			if (column < linelength)
			{
				temp->line = malloc(strlen(currentLineBuffer + column) + 1);
				strcpy(temp->line, currentLineBuffer + column + 1);
			}
			else
			{
				temp->line = malloc(1);
				temp->line[0] = 0;
			}
			CLRBUFF;
			strcpy(currentLineBuffer, temp->line);
			// Reset all the counters and reposition the cursor
			column = 0;
			line++;
			currline = currline->next;
			RedrawScreen(windowStart);
			break;
		case ctrl('U'): // Up Arrow
			if (currline->prev)
			{
				line--;
				free(currline->line);
				currline->line = malloc(strlen(currentLineBuffer) + 1);
				strcpy(currline->line, currentLineBuffer);
				currline = currline->prev;
				CLRBUFF;
				if (currline->line)
					strcpy(currentLineBuffer, currline->line);
				if (column > strlen(currentLineBuffer))
					column = strlen(currentLineBuffer);
				printf("%c[%d;%dH", ESC, line, column);
				if (line < 0)
				{
					windowStart = windowStart->prev;
					line++;
					RedrawScreen();
				}
			}
			break;
		case ctrl('D'): // Down Arrow
			if (currline->next)
			{
				line++;
				free(currline->line);
				currline->line = malloc(strlen(currentLineBuffer) + 1);
				strcpy(currline->line, currentLineBuffer);
				currline = currline->next;
				CLRBUFF;
				if (currline->line)
					strcpy(currentLineBuffer, currline->line);
				if (column > strlen(currentLineBuffer))
					column = strlen(currentLineBuffer);
				printf("%c[%d;%dH", ESC, line, column);
				if (line > WINDOWSIZE)
				{
					windowStart = windowStart->next;
					line--;
					RedrawScreen();
				}
			}
			break;
		case ctrl('L'): // Left Arrow
			if (column)
			{
				printf("%c[1D", ESC);
				column--;
			}
			break;
		case ctrl('R'): // Right Arrow
			if (column < 80 && column < strlen(currentLineBuffer))
			{
				column++;
				printf("%c[1C", ESC);
			}
			break;
		case 8: // BackSpace
			if (column)
			{
				int i;
				for (i = column; i < 80; i++)
					currentLineBuffer[i - 1] = currentLineBuffer[i];
				column--;
				printf("%c[1D", ESC);
				printf("%s", currentLineBuffer + column);
				printf("%c[%d;%dH", ESC, line, column);
			}
			break;
		default:
			if (mode == Insert)
			{
				int i;
				for (i = 80; i > column; i--)
					currentLineBuffer[i] = currentLineBuffer[i - 1];
			}
			currentLineBuffer[column] = c;
			printf("%s", currentLineBuffer + column);
			if (column < 80)
				column++;
			printf("%c[%d;%dH", ESC, line, column);
			break;
		}
	}
	free(currline->line);
	currline->line = malloc(strlen(currentLineBuffer) + 1);
	strcpy(currline->line, currentLineBuffer);

// Clear the screen
	printf("%c[2J", ESC);
	fflush(stdout);
	if (file != -1)
		sys_truncate(file, 0);
	else
		file = creat(argv[1]);
	currline = lines;
	if (file)
	{
		while (currline)
		{
			if (currline->line)
			{
				write(file, currline->line, strlen(currline->line));
				write(file, "\n", 1);
			}
			currline = currline->next;
		}
		close(file);
	}
	free(lines->line);
	free(lines);
	return 0;
}
