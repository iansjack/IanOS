#include <memory.h>
#include <kernel.h>

#define KBDINT       1

#define GETCHAR      1
#define KEYPRESS     2
#define GETKEY       3

#define LSHIFT       42
#define RSHIFT       54
#define ALT          56
#define CTRL		 29
#define CAPSLOCK     58
#define F1           59
#define F2           60
#define F3           61
#define F4           62
#define F5           63
#define F6           64
#define F7           65
#define F8           66

#define SHIFTED     1
#define LOCKED      2
#define CTRLED      4
#define ALTED       8

extern void switchConsole(unsigned char console);
extern struct MessagePort *KbdPort;

long kbBufStart;
long kbBufCurrent;
unsigned char kbBufCount;
unsigned char kbBuffer[128];
unsigned char currentBuffer;
struct Console consoles[8];

//==============================
// The unshifted keyboard table.
//==============================

unsigned char KbdTableU[] =
{ 0, 0 /*esc */, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
		8 /*backspace */, 0 /*tab */, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
		'o', 'p', '[', ']', 13, 0 /*ctrl */, 'a', 's', 'd', 'f', 'g', 'h', 'j',
		'k', 'l', ';', '\'', '`', 0 /*lshift */, '#', 'z', 'x', 'c', 'v', 'b',
		'n', 'm', ',', '.', '/', 0 /*rshift */, 0 /*sysreq */, 0 /*alt */, ' ',
		0 /*capslock */, 0 /*F1 */, 0 /*F2 */, 0 /*F3 */, 0 /*F4 */, 0 /*F5 */,
		0 /*F6 */, 0 /*F7 */, 0 /*F8 */, 0 /*F9 */, 0 /*F10 */, 0 /*numlock */,
		0 /*scrolllock */, 0 /*home */, 21 /*uarrow */, 0 /*pup */,
		0 /*numminus */, 12 /*larrow */, 0 /*num5 */, 18 /*rarrow */,
		0 /*numplus */, 0 /*end */, 4 /*darrow */, 0 /*pdown */, 0 /*ins */,
		127 /*del */, 0, 0, '\\', 0 /*F11 */, 0 /*F12 */
};

//==============================
// The shifted keyboard table.
//==============================

unsigned char KbdTableS[] =
{ 0, 0 /*esc */, '!', '"', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
		8 /*backspace */, 0 /*tab */, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
		'O', 'P', '{', '}', 13, 0 /*ctrl */, 'A', 'S', 'D', 'F', 'G', 'H', 'J',
		'K', 'L', ':', '@', '`', 0 /*lshift */, '#', 'Z', 'X', 'C', 'V', 'B',
		'N', 'M', '<', '>', '?', 0 /*rshift */, 0 /*sysreq */, 0 /*alt */, ' ',
		0 /*capslock */, 0 /*F1 */, 0 /*F2 */, 0 /*F3 */, 0 /*F4 */, 0 /*F5 */,
		0 /*F6 */, 0 /*F7 */, 0 /*F8 */, 0 /*F9 */, 0 /*F10 */, 0 /*numlock */,
		0 /*scrolllock */, 0 /*home */, 0 /*uarrow */, 0 /*pup */,
		'-' /*numminus */, 0 /*larrow */, '5' /*num5 */, 0 /*rarrow */,
		'+' /*numplus */, 0 /*end */, 0 /*darrow */, 0 /*pdown */, 0 /*ins */,
		127 /*del */, 0, 0, '|', 0 /*F11 */, 0 /*F12 */
};

//==============================
// The Ctrl keyboard table.
//==============================

unsigned char KbdTableC[] =
{ 0, 0 /*esc */, '!', '"', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
		8 /*backspace */, 0 /*tab */, 17, 23, 5, 18, 20, 25, 21, 9, 15, 16, '{',
		'}', 13, 0 /*ctrl */, 1, 19, 4, 6, 7, 8, 10, 11, 12, ':', '@', '`',
		0 /*lshift */, '#', 26, 24, 3, 22, 2, 14, 13, '<', '>', '?',
		0 /*rshift */, 0 /*sysreq */, 0 /*alt */, ' ', 0 /*capslock */,
		0 /*F1 */, 0 /*F2 */, 0 /*F3 */, 0 /*F4 */, 0 /*F5 */, 0 /*F6 */,
		0 /*F7 */, 0 /*F8 */, 0 /*F9 */, 0 /*F10 */, 0 /*numlock */,
		0 /*scrolllock */, 0 /*home */, 0 /*uarrow */, 0 /*pup */,
		'-' /*numminus */, 0 /*larrow */, '5' /*num5 */, 0 /*rarrow */,
		'+' /*numplus */, 0 /*end */, 0 /*darrow */, 0 /*pdown */, 0 /*ins */,
		127 /*del */, 0, 0, '|', 0 /*F11 */, 0 /*F12 */
};

//==========================================================
// Send a KEYPRESSED message to kbTaskCode
//==========================================================
void keyPressed()
{
	// Send a message to the keyboard task saying that a key has been pressed.
	struct Message *kbdMsg = (struct Message *) ALLOCMSG;

	kbdMsg->nextMessage = 0;
	kbdMsg->byte = KEYPRESS;
	SendMessage(KbdPort, kbdMsg);
	DeallocMem(kbdMsg);
}

void ProcessMsgQueue(struct Console *console)
{
	char temp;
	struct Message *tempMsg;

	// Check if there is a request on the current console's message queue.
	// Since the keypress came from the current console that is the one we look at.
	while (console->kbBufCount && console->MsgQueue)
	{
		// Take the first character from the buffer
		temp = console->kbBuffer[console->kbBufStart];
		console->kbBufCount--;
		console->kbBufStart++;
		tempMsg = console->MsgQueue;
		console->MsgQueue = tempMsg->nextMessage;

		// If it's a GETCHAR message, send the character back to the calling program
		// There should be no other requests, so anything else is discarded
		if (tempMsg->byte == GETCHAR)
		{
			struct MessagePort *tempPort = tempMsg->tempPort;
			tempMsg->nextMessage = 0;
			tempMsg->quad1 = 0L;
			tempMsg->byte = (unsigned char)temp;
			SendMessage(tempPort, tempMsg);
		}
		DeallocMem(tempMsg);
	}
}

//=====================================================
// This is the task that listens for keyboard requests.
//=====================================================
void kbTaskCode()
{
	unsigned char temp = 0;
	unsigned char modifier = 0;
	struct Message *KbdMsg;
	struct Console *currentCons;
	struct Message *tempMsg;
	int i;

	kprintf(1, 0, "Starting Keyboard Task");

	KbdPort = AllocMessagePort();

	for (i = 0; i < 4; i++)
	{
		consoles[i].kbBuffer = AllocUMem(128);
		consoles[i].kbBufStart = consoles[i].kbBufCurrent =
				consoles[i].kbBufCount = 0;
		consoles[i].MsgQueue = 0;
	}

	kbBufStart = 0;
	kbBufCurrent = 0;
	kbBufCount = 0;
	currentBuffer = 0;

	// enable keyboard + timer interrupt
	asm("mov $0b11111000, %al");
	asm("out %al, $0x21");

	KbdPort->waitingProc = (struct Task *) -1L;
	KbdPort->msgQueue = 0;
	while (1)
	{
		KbdMsg = (struct Message *) ALLOCMSG;
		ReceiveMessage(KbdPort, KbdMsg);
		switch (KbdMsg->byte)
		{
		case GETCHAR:
			// We are interested in the console that requested a character.
			// It may not be the current one.
			currentCons = &consoles[KbdMsg->quad1];
			tempMsg = currentCons->MsgQueue;
			if (!tempMsg)
				currentCons->MsgQueue = KbdMsg;
			else
			{
				while (tempMsg->nextMessage)
					tempMsg = tempMsg->nextMessage;
				tempMsg->nextMessage = KbdMsg;
			}
			break;

		case KEYPRESS:
			// We are interested in the console that sent the character.
			// It will always be the current console.
			currentCons = &(consoles[currentBuffer]);
			// Keep going as long as there are unprocessed characters.
			// I don't think there should ever be more than one, but it does no harm.
			while (kbBufCount)
			{
				temp = kbBuffer[kbBufStart];
				kbBufCount--;
				kbBufStart++;
				if ((kbBufStart) == 128)
					(kbBufStart) = 0;
				if (temp < 0x80)
				{
					// This is the case when the key is depressed
					switch (temp)
					{
					case LSHIFT:
					case RSHIFT:
						modifier |= SHIFTED;
						break;

					case CAPSLOCK:
						if (modifier & LOCKED)
							modifier &= ~LOCKED;
						else
							modifier |= LOCKED;
						break;

					case CTRL:
						modifier |= CTRLED;
						break;

					case F1:
						switchConsole(0);
						break;

					case F2:
						switchConsole(1);
						break;

					case F3:
						switchConsole(2);
						break;

					case F4:
						switchConsole(3);
						break;

					default:
						// Translate the keycode to a character
						if (modifier & CTRLED)
							temp = KbdTableC[temp];
						else if (modifier & SHIFTED)
							temp = KbdTableS[temp];
						else
							temp = KbdTableU[temp];
						if (modifier & LOCKED)
						{
							if ((temp >= 'A') && (temp <= 'Z'))
								temp += 0x20;
							else if ((temp >= 'a') && (temp <= 'z'))
								temp -= 0x20;
						}
						currentCons->kbBuffer[currentCons->kbBufCurrent] = (char) temp;
						currentCons->kbBufCount++;
						currentCons->kbBufCurrent++;
						break;
					}
				}
				else
				{
					// This is the case when the key is released. This only affects the
					// modifier keys (but not CapsLock)
					switch (temp & 0x7F)
					{
					case LSHIFT:
					case RSHIFT:
						modifier &= ~SHIFTED;
						break;
					case CTRL:
						modifier &= ~CTRLED;
						break;
					default:
						break;
					}
				}
			}

			// Process the console's message queue in case there are any messages waiting for this character
			ProcessMsgQueue(currentCons);
			DeallocMem(KbdMsg);
			break;

		default:
			// This shouldn't happen! Just throw away the message.
			DeallocMem(KbdMsg);
			break;
		}
	}
}
