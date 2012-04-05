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
#define CLRBUFF	for (count = 0; count < 80; count++) currentLineBuffer[count] = 0

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
	lines = malloc(sizeof(struct line));
	lines->next = 0;
	lines->prev = 0;
	lines->lineno = 0;
	currline = lines;

	int i;
	while (filesize)
	{
		while (buffer[i] != 0x0a && i < filesize)
			i++;
		buffer[i] = 0;
		currline->line = malloc(i + 1);
		strcpy(currline->line, buffer);
		buffer = buffer + i + 1;
		filesize -= i + 1;
		if (filesize)
		{
			i = 0;
			currline->next = malloc(sizeof(struct line));
			currline->next->prev = currline;
			currline = currline->next;
		}
	}
}

int main(int argc, char **argv)
{
	lines = 0;

	FD file;
	int line = 0;
	int column = 0;
	struct line *currline;
	char currentLineBuffer[80];
	int currentLineLength = 0;
	int count;

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
		}
		else
		{
			file = creat(argv[1]);
			lines = malloc(sizeof(struct line));
			lines->next = 0;
			lines->prev = 0;
			lines->lineno = 0;
			lines->line = malloc(80);
		}
		currline = lines;
		printf("%c[0;0H", ESC);
	}
	int done = 0;

	char c;
	if (currline->line)
		strcpy(currentLineBuffer, currline->line);
	else
		currentLineBuffer[0] = 0;

	currentLineLength = strlen(currentLineBuffer);
	while (!done)
	{
		c = getchar();
		if (c == ctrl('Q'))
			done = 1;
		else if (c == ctrl('M')) // Return
		{
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
			currentLineLength = strlen(currentLineBuffer);
			// Repaint whole screen
			printf("%c[2J", ESC);
			printf("%c[0;0H", ESC);
			struct line *temp2 = lines;
			while (temp2)
			{
				if (temp2->line)
					printf("%s\n", temp2->line);
				temp2 = temp2->next;
			}
			// Reset the cursor
			printf("%c[%d;0H", ESC, line);
		}
		else if (c == ctrl('U')) // Up Arrow
		{
			if (line)
			{
				line--;
				free(currline->line);
				currline->line = malloc(strlen(currentLineBuffer) + 1);
				strcpy(currline->line, currentLineBuffer);
				currline = currline->prev;
				CLRBUFF;
				strcpy(currentLineBuffer, currline->line);
				if (column > strlen(currentLineBuffer))
					column = strlen(currentLineBuffer);
				printf("%c[%d,%dH", ESC, line, column);
			}
		}
		else if (c == ctrl('D')) // Down Arrow
		{
			if (currline->next)
			{
				line++;
				free(currline->line);
				currline->line = malloc(strlen(currentLineBuffer) + 1);
				strcpy(currline->line, currentLineBuffer);
				currline = currline->next;
				CLRBUFF;
				strcpy(currentLineBuffer, currline->line);
				if (column > strlen(currentLineBuffer))
					column = strlen(currentLineBuffer);
				printf("%c[%d,%dH", ESC, line, column);
			}
		}
		else if (c == ctrl('L')) // Left Arrow
		{
			if (column)
			{
				printf("%c[1D", ESC);
				column--;
			}
		}
		else if (c == ctrl('R')) // Right Arrow
		{
			if (column < 80 && column < strlen(currentLineBuffer))
			{
				column++;
				printf("%c[1C", ESC);
			}
		}
		else
		{
			printf("%c", c);
			currentLineBuffer[column] = c;
			if (column < 80)
				column++;
			if (column == currentLineLength)
			{
				currentLineBuffer[column] = 0;
				currentLineLength++;
			}
		}
	}
	free(currline->line);
	currline->line = malloc(strlen(currentLineBuffer) + 1);
	strcpy(currline->line, currentLineBuffer);

// Clear the screen
	printf("%c[2J", ESC);
	if (file != -1)
	{
		close(file);
		unlink(argv[1]);
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
	}
	free(lines->line);
	free(lines);
	exit();
	return 0;
}
