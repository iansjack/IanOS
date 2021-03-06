#include <kernel.h>
#include <console.h>

//====================================================
// This is the task that listens for console requests.
//====================================================

extern unsigned char currentBuffer;
extern struct Console consoles[8];
extern long canSwitch;
extern struct MessagePort *ConsolePort;

char *VideoBuffer;
char *ConsoleBuffer;
struct Message ConsoleMsg;
unsigned char Mode;
struct Console *currCons;

void switchConsole(unsigned char console)
{
	asm("cli");
	if (currentBuffer != console)
	{
		int i;
		ConsoleBuffer = consoles[console].ConsoleBuffer;
		for (i = 0; i < 4096; i++)
			VideoBuffer[i] = ConsoleBuffer[i];
	}
	currentBuffer = console;
	currCons = &consoles[console];
	asm("sti");
	Position_Cursor(currCons->row, currCons->column);
}

void ScrollScreen(unsigned char console)
{
	short int row;
	short int column;

	asm("cli");
	canSwitch++;

	for (row = 1; row < 25; row++)
		for (column = 0; column < 80; column++)
		{
			ConsoleBuffer[160 * (row - 1) + 2 * column] = ConsoleBuffer[160
					* row + 2 * column];
			ConsoleBuffer[160 * (row - 1) + 2 * column + 1] = ConsoleBuffer[160
					* row + 2 * column + 1];
		}
	for (column = 0; column < 80; column++)
		ConsoleBuffer[160 * 24 + 2 * column] = ' ';
	if (currentBuffer == console)
	{
		for (row = 1; row < 25; row++)
			for (column = 0; column < 80; column++)
			{
				VideoBuffer[160 * (row - 1) + 2 * column] = VideoBuffer[160
						* row + 2 * column];
				VideoBuffer[160 * (row - 1) + 2 * column + 1] = VideoBuffer[160
						* row + 2 * column + 1];
			}
		for (column = 0; column < 80; column++)
			VideoBuffer[160 * 24 + 2 * column] = ' ';
	}
	currCons->row--;
	canSwitch--;
	asm("sti");
}

void ClrScr(unsigned char console)
{
	short int row;
	short int column;

	for (row = 0; row < 25; row++)
	{
		for (column = 0; column < 80; column++)
		{
			currCons->ConsoleBuffer[160 * row + 2 * column] = ' ';
			currCons->ConsoleBuffer[160 * row + 2 * column + 1] = 7;
			if (currentBuffer == console)
			{
				VideoBuffer[160 * row + 2 * column] = ' ';
				VideoBuffer[160 * row + 2 * column + 1] = 7;
			}
		}
	}
	currCons->row = currCons->column = 0;
}

void ClrEOL(unsigned char console)
{
	int i = currCons->column;

	while (i < 80)
	{
		ConsoleBuffer[160 * currCons->row + 2 * i] = ' ';
		if (currentBuffer == console)
			VideoBuffer[160 * currCons->row + 2 * i] = ' ';
		i++;
	}
}

void PrintChar(char c, unsigned char console)
{
	int n;

	switch (c)
	{
	case 0:
		break;

	case BACKSPACE:
		if (currCons->column > 0)
			currCons->column--;
		break;

	case TAB:
		for (n = 0; n < TABSIZE; n++)
			PrintChar(' ', console);
		break;

	case SOL:
		currCons->column = 0;
		break;

	case LF:
		currCons->column = 0;
		currCons->row++;
		if (currCons->row == 25)
			ScrollScreen(console);
		break;

	default:
		currCons->ConsoleBuffer[160 * currCons->row + 2 * currCons->column] = c;
		if (currCons->colour == REVERSE)
			currCons->ConsoleBuffer[160 * currCons->row + 2 * currCons->column
					+ 1] = 0x70;
		if (currentBuffer == console)
		{
			VideoBuffer[160 * currCons->row + 2 * currCons->column] = c;
			if (currCons->colour == REVERSE)
				VideoBuffer[160 * currCons->row + 2 * currCons->column + 1] =
						0x70;
		}
		currCons->column++;
		if (currCons->column == 80)
		{
			currCons->column = 0;
			currCons->row++;
			if (currCons->row == 25)
				ScrollScreen(console);
		}
		break;
	}
	Position_Cursor(currCons->row, currCons->column);
}

void ProcessChar(char c, unsigned char console)
{
	int n1, n2;

	switch (Mode)
	{
	case NORMAL_MODE:
		if (c == ESC)
			Mode = ESC_MODE;
		else
			PrintChar(c, console);
		break;

	case ESC_MODE:
		if (c == '[')
			Mode = ESCBR_MODE;
		else if (c == '#')
			Mode = ESCHASH_MODE;
		else if (c == 'D')
		{
			ScrollScreen(console);
			Mode = NORMAL_MODE;
		}
		else
		{
			PrintChar(c, console);
			Mode = NORMAL_MODE;
		}
		break;

	case ESCBR_MODE:
		if (c >= '0' && c <= '9')
		{
			n1 = c - '0';
			Mode = ESCBRNUM_MODE;
		}
		else if (c == '?')
			Mode = ESCBRQ_MODE;
		else
		{
			PrintChar(c, console);
			Mode = NORMAL_MODE;
		}
		break;

	case ESCBRNUM_MODE:
		if (c >= '0' && c <= '9')
		{
			n1 = 10 * n1 + c - '0';
			break;
		}
		else if (c == ';')
		{
			Mode = ESCBRNUMSEMI_MODE;
			n2 = 0;
			break;
		}
		else if (c == 'A')
		{
			currCons->row -= n1;
			if (currCons->row < 0)
				currCons->row = 0;
		}
		else if (c == 'B')
		{
			currCons->row += n1;
			if (currCons->row > 24)
				currCons->row = 24;
		}
		else if (c == 'C')
		{
			currCons->column += n1;
			if (currCons->column > 79)
				currCons->column = 79;
		}
		else if (c == 'D')
		{
			currCons->column -= n1;
			if (currCons->column < 0)
				currCons->column = 0;
		}
		else if (n1 == 2 && c == 'J')
			ClrScr(console);
		else if (n1 == 0 && c == 'K')
			ClrEOL(console);
		Mode = NORMAL_MODE;
		break;

	case ESCHASH_MODE:
		if (c >= '0' && c <= '9')
		{
			n1 = c - '0';
			if (n1 >= 0 && n1 <= 3)
				PidToTask(ConsoleMsg.pid)->console = n1;
		}
		else
			PrintChar(c, console);
		Mode = NORMAL_MODE;
		break;

	case ESCBRQ_MODE:
		if (c == '5')
			Mode = ESCBRQ5_MODE;
		else
		{
			PrintChar(c, console);
			Mode = NORMAL_MODE;
		}
		break;

	case ESCBRQ5_MODE:
		if (c == 'h')
			currCons->colour = REVERSE;
		else if (c == 'l')
			currCons->colour = NORMAL;
		else
			PrintChar(c, console);
		Mode = NORMAL_MODE;
		break;

	case ESCBRNUMSEMI_MODE:
		if (c >= '0' && c <= '9')
			n2 = n2 * 10 + c - '0';
		else if (c == 'H')
		{
			currCons->row = n1;
			currCons->column = n2;
			Mode = NORMAL_MODE;
		}
		break;

	default:
		PrintChar(c, console);
		Mode = NORMAL_MODE;
		break;
	}
	Position_Cursor(currCons->row, currCons->column);
}

void consoleTaskCode()
{
	int i;
	char *s;

	kprintf(2, 0, "Starting Console Task");
	Mode = NORMAL_MODE;
	ConsolePort = AllocMessagePort();

	VideoBuffer = (char *) 0x80000B8000;
	ConsolePort->waitingProc = (struct Task *) -1L;
	ConsolePort->msgQueue = 0;

	for (i = 0; i < 4; i++)
	{
		consoles[i].ConsoleBuffer = AllocKMem(4096);
		consoles[i].column = consoles[i].row = 0;
		consoles[i].colour = NORMAL;
	}

	while (1)
	{
		unsigned char console;

		ReceiveMessage(ConsolePort, &ConsoleMsg);
		console = (unsigned char)ConsoleMsg.quad2;
		currCons = &(consoles[console]);
		ConsoleBuffer = currCons->ConsoleBuffer;

		switch (ConsoleMsg.byte)
		{
		case WRITECHAR:
			ProcessChar((char) ConsoleMsg.quad1, (unsigned char) ConsoleMsg.quad2);
			break;

		case WRITESTR:
			s = (char *) ConsoleMsg.quad1;
			console = (unsigned char) ConsoleMsg.quad2;
			while (*s != 0)
				ProcessChar(*s++, console);
			DeallocMem((void *) ConsoleMsg.quad1);
			break;

		default:
			break;
		}
	}
}
