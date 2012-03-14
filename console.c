#include "memory.h"
#include "kernel.h"
#include "console.h"

//====================================================
// This is the task that listens for console requests.
//====================================================

extern unsigned char currentBuffer;
extern struct Console consoles[8];
extern long canSwitch;

unsigned char *VideoBuffer;
unsigned char *ConsoleBuffer;
struct Message ConsoleMsg;
unsigned char Mode;
struct Console * currCons;

void switchConsole(long console)
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
   	asm("sti");
}

void ScrollScreen(long console)
{
	asm("cli");
	canSwitch++;
   	short int row;
   	short int column;

   	for (row = 1; row < 25; row++)
      	for (column = 0; column < 80; column++)
		{
         	ConsoleBuffer[160 * (row - 1) + 2 * column] = ConsoleBuffer[160 * row + 2 * column];
         	ConsoleBuffer[160 * (row - 1) + 2 * column + 1] = ConsoleBuffer[160 * row + 2 * column + 1];
		}
   for (column = 0; column < 80; column++)
      ConsoleBuffer[160 * 24 + 2 * column] = ' ';
   	if (currentBuffer == console)
   	{
		for (row = 1; row < 25; row++)
			for (column = 0; column < 80; column++)
	   		{
				VideoBuffer[160 * (row - 1) + 2 * column] = VideoBuffer[160 * row + 2 * column];
				VideoBuffer[160 * (row - 1) + 2 * column + 1] = VideoBuffer[160 * row + 2 * column + 1];
		 	}
      	for (column = 0; column < 80; column++)
  			VideoBuffer[160 * 24 + 2 * column] = ' ';
   	}
	currCons->row--;
   	canSwitch--;
   	asm("sti");
}

void ClrScr(long console)
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
}

void ClrEOL(long console)
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

void ProcessChar(unsigned char c)
{
	int n1, n2;
   	long console = PidToTask(ConsoleMsg.pid)->console;
	
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
				break;
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
}

void PrintChar(unsigned char c, long console)
{
   	switch (c)
	{
   		case 0:
      		break;

   		case BACKSPACE:
      		if (currCons->column > 0)
         		currCons->column--;
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
         		currCons->ConsoleBuffer[160 * currCons->row + 2 * currCons->column + 1] = 0x70;
      		if (currentBuffer == console)
      		{
         		VideoBuffer[160 * currCons->row + 2 * currCons->column] = c;
         		if (currCons->colour == REVERSE)
            		VideoBuffer[160 * currCons->row + 2 * currCons->column + 1] = 0x70;
      		}
      		currCons->column++;
      		if (currCons->column == 80)
      		{
         		currCons->column = 0;
         		currCons->row++;
         		if (currCons->row == 25)
            		ScrollScreen(console);
      		}
	}
}

void consoleTaskCode()
{
	KWriteString("Starting Console Task", 2, 0);
	Mode = NORMAL_MODE;

   	VideoBuffer = (char *) 0xB8000;
   	((struct MessagePort *) ConsolePort)->waitingProc = (struct Task *) -1L;
   	((struct MessagePort *) ConsolePort)->msgQueue = 0;

   	int i;
   	for (i = 0; i < 4; i++)
   	{
      	consoles[i].ConsoleBuffer = AllocKMem(4096);
      	consoles[i].column = consoles[i].row = 0;
      	consoles[i].colour = NORMAL;
   	}

   	unsigned char *s;

   	while (1)
   	{
      	short row;
      	short column;

      	ReceiveMessage((struct MessagePort *) ConsolePort, &ConsoleMsg);
      	long console = ConsoleMsg.quad2;
     	currCons = &(consoles[console]);
      	ConsoleBuffer = currCons->ConsoleBuffer;

		switch (ConsoleMsg.byte)
		{
      		case WRITECHAR:
         		ProcessChar((unsigned char) ConsoleMsg.quad);
         		break;

      		case WRITESTR:
         		s = (unsigned char *) ConsoleMsg.quad;
         		while (*s != 0)
            		ProcessChar(*s++);
         		DeallocMem((void *) ConsoleMsg.quad);
         		break;

      		default:
         		break;
		}
	}
}
