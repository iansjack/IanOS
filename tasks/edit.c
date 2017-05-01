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
#define RVideo			printf("%c[?5h", ESC)
#define NVideo			printf("%c[?5l", ESC)
#define Overwrite	0
#define Insert		1
#define WINDOWSIZE	20

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
char *filename;

int ReadFile(int file)
{
	struct line *currline;
	size_t filesize;
	char *buffer, *bufferstart, *line;
	int linecount = 0;

	// First read the whole file into a buffer
	struct stat info;
	fstat(file, &info);
	filesize = (size_t) (info.st_size);
	buffer = malloc(filesize + 1);
	bufferstart = buffer;
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

	char *nextret;
	line = malloc(256);
	while (filesize)
	{
		nextret = strstr(buffer, "\n");
		filesize -= nextret - buffer + 1;
		strncpy(line, buffer, nextret - buffer);
		line[nextret - buffer] = 0;
		buffer = nextret + 1;
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
	free(line);
	free(bufferstart);
	return 0;
}

void PrintStatusLines()
{
	printf("%c[0;0H", ESC);
	RVideo;
	printf("  Ian's edit 0.0.1                  File: %s                     Modified  ", filename);
	printf("%c[23;0H", ESC);
	RVideo;
	printf("^G");
	NVideo;
	printf(" Get Help  ");
	RVideo;
	printf("^O");
	NVideo;
	printf(" Write Out ");
	RVideo;
	printf("^W");
	NVideo;
	printf(" Where Is  ");
	RVideo;
	printf("^K");
	NVideo;
	printf(" Cut Text   ");
	RVideo;
	printf("^T");
	NVideo;
	printf(" To Spell  ");
	RVideo;
	printf("^Y");
	NVideo;
	printf(" Prev Page ");
	printf("%c[24;0H", ESC);
	RVideo;
	printf("^X");
	NVideo;
	printf(" Exit      ");
	RVideo;
	printf("^R");
	NVideo;
	printf(" Read File ");
	RVideo;
	printf("^\\");
	NVideo;
	printf(" Replace   ");
	RVideo;
	printf("^U");
	NVideo;
	printf(" Uncut Text ");
	RVideo;
	printf("^C");
	NVideo;
	printf(" Cur Pos   ");
	RVideo;
	printf("^V");
	NVideo;
	printf(" Next Page ");
	NVideo;
	printf("%c[%d;%dH", ESC, line, column);
}

void RedrawScreen()
{
	struct line *tempcurrline = lines;
	Clear_Screen;
	printf("%c[2;0H", ESC);
	tempcurrline = windowStart;
	if (!windowStart)
		return;
	for (count = 0; count < WINDOWSIZE; count++)
	{
		if (tempcurrline->line)
			printf("%s\n", tempcurrline->line);
		if (!tempcurrline->next)
			break;
		tempcurrline = tempcurrline->next;
	}
	PrintStatusLines();
}

int main(int argc, char **argv)
{
	int file;
	struct line *currline, *temp;
	unsigned char currentLineBuffer[80], c;
	int count, done, linelength;

	if (argc ==1)
	{
		printf("Syntax 'edit <filename>'\n");
		return 0;
	}

	lines = 0;
	line = 2;
	column = 0;
	mode = Insert;
	Clear_Screen;
	fflush(stdout);

	if (argc == 2)
	{
		file = open(argv[1], O_RDWR | O_CREAT);
		filename = argv[1];
		if (ReadFile(file))
			return 1;
		windowStart = lines;
		RedrawScreen();
		fflush(stdout);
		currline = lines;
	}
	done = 0;
	PrintStatusLines();
	Clear_Buffer;
	if (currline->line)
		strcpy(currentLineBuffer, currline->line);

	while (!done)
	{
		c = getchar();
		switch (c)
		{
		case ctrl('X'):
		case ctrl('Q'):
			done = 1;
			break;
		case ctrl('I'):
			if (mode == Insert)
				mode = Overwrite;
			else
				mode = Insert;
			PrintStatusLines();
			break;
		case ctrl('M'): // Return
			;
			linelength = (int) strlen(currentLineBuffer);
			// Copy the line up to cursor back to currline->line
			free(currline->line);
			currline->line = malloc(strlen(currentLineBuffer - column) + 1);
			strncpy(currline->line, currentLineBuffer, column);
			currline->line[column] = 0;
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
				temp->line = malloc(strlen(currentLineBuffer - column) + 1);
				strcpy(temp->line, currentLineBuffer + column);
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
			Position_Cursor;
			RedrawScreen();
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
				if (line < 2)
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
				if (line > WINDOWSIZE + 1)
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
				strcpy(currline->line, currentLineBuffer);
			}
			else
			{
				if (currline->prev)
				{
					column = strlen(currline->prev->line);
					line--;
					strcat(currline->prev->line, currline->line);
					strcpy(currentLineBuffer, currline->prev->line);
					currline->prev->next = currline->next;
					currline->next->prev = currline->prev;
					struct line *temp = currline->prev;
					free(currline->line);
					free(currline);
					currline = temp;
				}
			}
			Position_Cursor;
			RedrawScreen();
			break;
		case 127: // Delete
				if (column)
				{
					int i;
					for (i = column; i < 80 - 1; i++)
						currentLineBuffer[i] = currentLineBuffer[i + 1];
					printf("%s ", currentLineBuffer + column);
					Position_Cursor;
				}
				break;
		case ctrl('V'):
		case 128: // PgDn
			if (currline->line)
				free(currline->line);
			currline->line = (char *) malloc(strlen(currentLineBuffer) + 1);
			strcpy(currline->line, currentLineBuffer);
			for (int i = 0; i < 17; i++)
			{
				if (!currline->next) break;
				currline = currline->next;
				if (windowStart->next)
					windowStart = windowStart->next;
			}
			Clear_Buffer;
			if (currline->line)
					strcpy(currentLineBuffer, currline->line);
			if (column > (int) strlen(currentLineBuffer))
				column = (int) strlen(currentLineBuffer);
			RedrawScreen();
			Position_Cursor;
			break;
		case ctrl('Y'):
		case 129: // PgUp
			if (currline->line)
				free(currline->line);
			currline->line = malloc(strlen(currentLineBuffer) + 1);
			strcpy(currline->line, currentLineBuffer);
			for (int i = 0; i < 17; i++)
			{
				if (!currline->prev) break;
				currline = currline->prev;
				if (windowStart->prev)
					windowStart = windowStart->prev;
				else
					line--;
			}
			Clear_Buffer;
			if (currline->line)
				strcpy(currentLineBuffer, currline->line);
			if (column > (int) strlen(currentLineBuffer))
				column = (int) strlen(currentLineBuffer);
			RedrawScreen();
			Position_Cursor;
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
