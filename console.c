#include "memory.h"
#include "kernel.h"
#include "console.h"

//====================================================
// This is the task that listens for console requests.
//====================================================

extern unsigned char currentBuffer;
extern struct Console consoles[8];

unsigned char  *VideoBuffer;
struct Message ConsoleMsg;

void switchConsole ( long console )
{
    if ( currentBuffer != console )
    {
        int i;
        for ( i = 0; i < 4096; i++ )
            VideoBuffer[i] = consoles[console].ConsoleBuffer[i];
    }
    currentBuffer = console;
}


void scrollscreen ( long console )
{
    short int row;
    short int column;
    struct Console * currCons = & ( consoles[console] );

    for ( row = 1; row < 25; row++ )
        for ( column = 0; column < 80; column++ )
            currCons->ConsoleBuffer[160 * ( row - 1 ) + 2 * column] = currCons->ConsoleBuffer[160 * row + 2 * column];
    for ( column = 0; column < 80; column++ )
        currCons->ConsoleBuffer[160 * 24 + 2 * column] = ' ';
    if ( currentBuffer == console )
    {
        for ( row = 1; row < 25; row++ )
            for ( column = 0; column < 80; column++ )
                VideoBuffer[160 * ( row - 1 ) + 2 * column] = VideoBuffer[160 * row + 2 * column];
        for ( column = 0; column < 80; column++ )
            VideoBuffer[160 * 24 + 2 * column] = ' ';
    }
}

void printchar ( unsigned char c )
{
    long console = ConsoleMsg.quad2;
    struct Console *currCons = & ( consoles[console] );

    switch ( c )
    {
    case 0:
        break;

    case BACKSPACE:
        if ( currCons->column > 0 )
        {
            currCons->column--;
        }
        break;

    case SOL:
        currCons->column = 0;
        break;

    case CR:
        currCons->column = 0;
        currCons->row++;
        if ( currCons->row == 25 )
        {
            scrollscreen ( console );
            currCons->row--;
        }
        break;

    default:
        currCons->ConsoleBuffer[160 * currCons->row + 2 * currCons->column] = c;
        if ( currentBuffer == console )
            VideoBuffer[160 * currCons->row + 2 * currCons->column] = c;
        currCons->column++;
        if ( currCons->column == 80 )
        {
            currCons->column = 0;
            currCons->row++;
            if ( currCons->row == 25 )
            {
                scrollscreen ( console );
                currCons->row--;
            }
        }
    }
}

void consoleTaskCode()
{

    VideoBuffer = ( char * ) 0xB8000;
    ( ( struct MessagePort * ) ConsolePort )->waitingProc = ( struct Task * )-1L;
    ( ( struct MessagePort * ) ConsolePort )->msgQueue    = 0;

    int i;
    for ( i = 0; i < 8; i++ )
    {
        consoles[i].ConsoleBuffer = AllocKMem ( 4096 );
        consoles[i].column = consoles[i].row = 0;
    }

    unsigned char *s;

    while ( 1 )
    {
        short row;
        short column;

        ReceiveMessage ( ( struct MessagePort * ) ConsolePort, &ConsoleMsg );
        long console = ConsoleMsg.quad2;
        struct Console * currCons = & ( consoles[console] );
        switch ( ConsoleMsg.byte )
        {
        case WRITECHAR:
            printchar ( ( unsigned char ) ConsoleMsg.quad );
            break;

        case WRITESTR:
            s = ( unsigned char * ) ConsoleMsg.quad;
            while ( *s != 0 )
            {
                printchar ( *s );
                s++;
            }
            DeallocMem ( ( void * ) ConsoleMsg.quad );
            break;

        case SETCURSOR:
            currCons->row = ConsoleMsg.quad;
            currCons->column = ConsoleMsg.quad3;
            break;

        case CLRSCR:
            for ( row = 0; row < 25; row++ )
            {
                for ( column = 0; column < 80; column++ )
                {
                    currCons->ConsoleBuffer[160 * row + 2 * column] = ' ';
                    currCons->ConsoleBuffer[160 * row + 2 * column + 1] = 7;
                    if ( currentBuffer == console )
                    {
                        VideoBuffer[160 * row + 2 * column]     = ' ';
                        VideoBuffer[160 * row + 2 * column + 1] = 7;
                    }
                }
            }
            currCons->column = currCons->row = 0;
            break;

        case CLREOL:
            i = currCons->column;
            while ( i < 80 )
            {
                currCons->ConsoleBuffer[160 * currCons->row + 2 * i] = ' ';
                if ( currentBuffer == console )
                    VideoBuffer[160 * currCons->row + 2 * i] = ' ';
                i++;
            }

        default:
            break;
        }
    }
}
