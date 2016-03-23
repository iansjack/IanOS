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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

void sys_truncate(int, int); // Defined in sys_truncate.s

#define ctrl(x) x - 0x40
#define ESC	27
#define Clear_Buffer	for (count = 0; count < 80; count++) currentLineBuffer[count] = 0
#define Clear_Screen	printf("%c[2J", ESC)
#define Position_Cursor printf("%c[%d;%dH", ESC, line, column)
#define Overwrite	0
#define Insert		1
#define WINDOWSIZE	23

struct line
{
	struct line *next;
	struct line *prev;
	char *line;
	int lineno;
};

struct line *lines;
struct line *windowStart;
int line, column;
char mode;
int count;

int ReadFile(int file)
{
	struct line *currline;
	size_t filesize;
	char *buffer, *line;
	int linecount = 0;

	// First read the whole file into a buffer
	struct stat info;
	fstat(file, &info);
	filesize = (size_t) (info.st_size);
	buffer = malloc(filesize + 1);
	if (!buffer)
	{
		printf("Buffer allocation failed.\n");
		close(file);
		return 1;
	}
	(void) read(file, buffer, filesize);
	buffer[filesize] = 0;

	// Parse the buffer into separate lines
	lines = malloc(sizeof(struct line));
	if (!lines)
	{
		printf("Allocation of lines failed.\n");
		close(file);
		return 1;
	}
	lines->next = 0;
	lines->prev = 0;
	lines->lineno = 0;
	currline = lines;

	line = strtok(buffer, "\n");
	while (line)
	{
		currline->line = malloc(strlen(line) + 1);
		if (!currline->line)
		{
			printf("Allocation of currline->line failed.\n");
			close(file);
			return 1;
		}
		strcpy(currline->line, line);
		linecount++;
		currline->lineno = linecount;
		line = strtok(NULL, "\n");
		if (line)
		{
			currline->next = malloc(sizeof(struct line));
			if (!currline->next)
			{
				printf("Allocation of currline->next failed for line %d.\n", linecount);
				printf("Errno: %d\n", errno);
				close(file);
				return 1;
			}
			currline->next->prev = currline;
			currline = currline->next;
		}
	}
	free(buffer);
	return 0;
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
	Clear_Screen;
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
	int file;
	struct line *currline, *temp;
	char currentLineBuffer[80], c;
	int count, done, linelength;

	if (argc ==1)
	{
		printf("Syntax 'edit <filename>'\n");
		return 0;
	}

	lines = 0;
	line = 0;
	column = 0;
	mode = Insert;
	Clear_Screen;
	fflush(stdout);

	if (argc == 2)
	{
		file = open(argv[1], O_RDWR | O_CREAT);
		if (ReadFile(file))
			return 1;
		windowStart = lines;
		RedrawScreen();
		fflush(stdout);
		currline = lines;
	}
	done = 0;
	PrintStatusLine();
	Clear_Buffer;
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
			linelength = (int) strlen(currentLineBuffer);
			currentLineBuffer[column] = 0;
			// Copy the line up to cursor back to currline->line
			free(currline->line);
			currline->line = malloc(strlen(currentLineBuffer) + 1);
			currentLineBuffer[column] = 0;
			strcpy(currline->line, currentLineBuffer);
			// Create a new struct line for the new line
			temp = malloc(sizeof(struct line));
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
			Clear_Buffer;
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
				Clear_Buffer;
				if (currline->line)
					strcpy(currentLineBuffer, currline->line);
				if (column > (int) strlen(currentLineBuffer))
					column = (int) strlen(currentLineBuffer);
				Position_Cursor;
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
				currline->line = (char *) malloc(strlen(currentLineBuffer) + 1);
				strcpy(currline->line, currentLineBuffer);
				currline = currline->next;
				Clear_Buffer;
				if (currline->line)
					strcpy(currentLineBuffer, currline->line);
				if (column > (int) strlen(currentLineBuffer))
					column = (int) strlen(currentLineBuffer);
				Position_Cursor;
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
			if (column < 80 && column < (int) strlen(currentLineBuffer))
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
				printf("%c[1D", ESC);						// Cursor left
				printf("%s ", currentLineBuffer + column);
				Position_Cursor;
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
			Position_Cursor;
			break;
		}
		fflush(stdout);
	}
	free(currline->line);
	currline->line = malloc(strlen(currentLineBuffer) + 1);
	strcpy(currline->line, currentLineBuffer);
	Clear_Screen;
	fflush(stdout);
	sys_truncate(file, 0);
	currline = lines;
	if (file)
	{
		while (currline)
		{
			if (currline->line)
			{
				(void) write(file, currline->line, strlen(currline->line));
				(void) write(file, "\n", 1);
			}
			currline = currline->next;
		}
		close(file);
	}
	free(lines->line);
	free(lines);
	return 0;
}
