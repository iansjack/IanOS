#include <kernel.h>

typedef long unsigned int size_t;

extern struct Task *currentTask;

/* extern long sec, min, hour, day, month, year, unixtime;

 void PrintClock()
 {
 kprintf(0, 63, "                 ");
 //	kprintf(0, 63, "%2d/%02d/%02d %2d:%02d:%02d", day, month, year, hour, min,
 //			sec);
 kprintf(0, 63, "%d", unixtime);
 }

 void setclock()
 {
 struct tm tm;
 tm.tm_sec = sec;
 tm.tm_min = min;
 tm.tm_hour = hour;
 tm.tm_mday = day;
 tm.tm_mon = month - 1;
 tm.tm_year = year + 100;
 tm.tm_isdst = -1;
 //tm.tm_gmtoff = 0;
 unixtime = (long) mktime(&tm);
 }
 */

//===========================================================================
// A kernel library function to write a null-terminated string to the screen.
//===========================================================================
void KWriteString(char *str, int row, int col)
{
	char *VideoBuffer = (char *) 0x80000B8000;

	int temp = 160 * row + 2 * col;
	int i = 0;
	while (str[i] != 0)
	{
		VideoBuffer[temp + 2 * i] = str[i];
		i++;
	}
}

//==========================================================================
// A kernel library function to write a quad character to the screen.
//==========================================================================
void KWriteHex(long c, int row) //, int col)
{
	char *VideoBuffer = (char *) 0x80000B8000;
	int i;
	char hi;

	for (i = 0; i < 8; i++)
	{
		char lo = (char) (c & 0xF);
		lo += 0x30;
		if (lo > 0x39)
		{
			lo += 7;
		}
		hi = (char) ((c & 0xF0) >> 4);
		hi += 0x30;
		if (hi > 0x39)
		{
			hi += 7;
		}
		VideoBuffer[160 * row + 28 - 4 * i] = hi;
		VideoBuffer[160 * row + 30 - 4 * i] = lo;
		c = c >> 8;
	}
}


char *NameToFullPath(char *name)
{
	char *S = AllocKMem(
			(size_t) (strlen(name) + strlen(currentTask->currentDirName) + 3));

	// "." is a very special case. It changes nothing.
	if (!strcmp(name, "."))
	{
		strcpy(S, currentTask->currentDirName);
		return S;
	}
	// The special case ".."
	if (!strcmp(name, ".."))
	{
		if (!strcmp(currentTask->currentDirName, "/"))
		{
			strcpy(S, "/");
			return S;
		}
		else
		{
			strcpy(S, currentTask->currentDirName);
			while (S[strlen(S) - 1] != '/')
				S[strlen(S) - 1] = 0;
			S[strlen(S) - 1] = 0;
			if (strlen(S) == 0)
				S[0] = '/';
		}
	}
	else
	{
		// The special case "./file"
		if (name[0] == '.' && name[1] == '/')
			name += 2;
		// The special case "../file"
		if (name[0] == '.' && name[1] == '.' && name[2] == '/')
		{
			name += 3;
			strcpy(S, currentTask->currentDirName);
			while (S[strlen(S) - 1] != '/')
				S[strlen(S) - 1] = 0;
			strcat(S, name);
		}
		else
		{
			strcpy(S, "");
			if (!strcmp(name, "/"))
				strcpy(S, "/");
			else
			{
				if (name[0] == '/')
					strcpy(S, name);
				else
				{
					if (strcmp(currentTask->currentDirName, "/"))
						strcpy(S, currentTask->currentDirName);
					strcat(S, "/");
					strcat(S, name);
				}
			}
		}
	}
	return S;
}

#include <stdarg.h>

int kprintf(int row, int column, char *s, ...)
{
	char sprocessed[256];
	int i = 0;
	int j = 0;
	int k = 0;
	short minwidth = 0;
	short zeropadding = 0;
	short indicator = 0;

	va_list ap;
	va_start(ap, s);

	for (i = 0; i < 256; i++)
		sprocessed[i] = 0;

	i = 0;

	while (s[i])
	{
		minwidth = 0;
		zeropadding = 0;
		indicator = 0;
		k = 0;

		if (s[i] != '%')
		{
			sprocessed[j] = s[i];
			i++;
			j++;
		}
		else
		{
			i++;
			if (s[i] == '#')
			{
				indicator = 1;
				i++;
			}
			if (s[i] == '0')
			{
				zeropadding = 1;
				i++;
			}
			if ('0' < s[i] && s[i] <= '9') // A width specifier
			{
				minwidth = s[i] - '0';
				i++;
			}
			switch (s[i])
			{
			int c, number;
			char *s1;
			char buffer[20];

		case 'c':
			while (minwidth-- > 1)
				sprocessed[j++] = ' ';
			c = va_arg(ap, int);
			sprocessed[j++] = c;
			break;
		case 's':
			s1 = va_arg(ap, char *);
			while (minwidth-- > strlen(s1))
				sprocessed[j++] = ' ';
			while (s1[k])
				sprocessed[j++] = s1[k++];
			break;
		case 'd':
			number = va_arg(ap, int);
			intToAsc(number, buffer, 20);
			for (k = 0; k < 20; k++)
				if (20 - k > minwidth)
				{
					if (buffer[k] != ' ')
						sprocessed[j++] = buffer[k];
				}
				else
				{
					if (zeropadding && buffer[k] == ' ')
						buffer[k] = '0';
					sprocessed[j++] = buffer[k];
				}
			break;
		case 'x':
			;
			number = va_arg(ap, int);
			intToHAsc(number, buffer, 20);
			if (indicator)
			{
				sprocessed[j++] = '0';
				sprocessed[j++] = 'x';
			}
			for (k = 0; k < 20; k++)
				if (20 - k > minwidth)
				{
					if (buffer[k] != ' ')
						sprocessed[j++] = buffer[k];
				}
				else
				{
					if (zeropadding && buffer[k] == ' ')
						buffer[k] = '0';
					sprocessed[j++] = buffer[k];
				}
			break;
		default:
			break;
			}
			i++;
		}
	}
	va_end(ap);
	KWriteString(sprocessed, row, column);
	return 0;
}

char *strrchr(char *string, char delimiter)
{
	int n = strlen(string);
	int i;
	for (i = n - 1; i >= 0; i--)
		if (string[i] == delimiter)
			break;
	return string + i;
}

memset(char *address, char value, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		address[i] = value;
}

memcpy(char *dest, char *source, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		dest[i] = source[i];
}

long strspn(char *s1, char *s2)
{
	long ret = 0;
	while (*s1 && strchr(s2, *s1++))
		ret++;
	return ret;
}

long strcspn(char *s1, char *s2)
{
	long ret = 0;
	while (*s1)
		if (strchr(s2, *s1))
			return ret;
		else
			s1++, ret++;
	return ret;
}

char *strtok(char * str, char * delim)
{
	static char* p = 0;
	if (str)
		p = str;
	else if (!p)
		return 0;
	str = p + strspn(p, delim);
	p = str + strcspn(str, delim);
	if (p == str)
		return p = 0;
	p = *p ? *p = 0, p + 1 : 0;
	return str;
}


//==================================================================
// Finds first occurrence of c in string.
// Returns pointer to that occurrence or 0 if not found
//==================================================================
char *strchr(char *string, char c)
{
	long i = 0;
	while (string[i])
	{
		if (string[i] == c)
			return string + i;
		i++;
	}
	return 0;
}

//================================================================
// Returns the length of string
//================================================================
long strlen(char *string)
{
	int i = 0;
	while (string[i++])
		;
	return i - 1;
}

//===============================================================
// Compares s1 and s2 up to length characters.
// Returns 0 if they are equal, non-zero otherwise.
//===============================================================
long strncmp(char *s1, char *s2, size_t length)
{
	size_t count;
	short done = 0;

	for (count = 0; count < length; count++)
	{
		if (s1[count] != s2[count])
		{
			done = 1;
			break;
		}
	}
	if (done)
		return (1);
	else
		return (0);
}

//=======================================================
// Copy null-terminates string s1 to s2
//=======================================================
char *strcpy(char *destination, char *source)
{
	char *retval = destination;
	while (*source)
	{
		*destination++ = *source++;
	}
	*destination = 0;
	return retval;
}

//===============================================================
// Compares s1 and s2.
// Returns 0 if they are equal, non-zero otherwise.
//===============================================================
long strcmp(char *s1, char *s2)
{
	if (s1 == s2)
		return 0;
	long retval = 1;
	int count = 0;
	while (s1[count] == s2[count])
	{
		if (s1[count++] == 0)
			retval = 0;
	}
	return retval;
}

//===========================================================
// Concatenates s2 to the end of s1.
// Returns s1
//===========================================================
char *strcat(char *s1, char *s2)
{
	int n = strlen(s1);
	strcpy(s1 + n, s2);
	return s1;
}


//=================================================================
// Convert the integer in i to its ASCII representation in buffer
// The integer is of length n
//=================================================================
int intToAsc(unsigned int i, char *buffer, int len)
{
	int count;

	for (count = 0; count < len; count++)
		buffer[count] = ' ';
	count = len - 1;
	do
	{
		buffer[count] = '0' + i % 10;
		i = i / 10;
		count--;
	} while (i > 0);
	return 0;
}

//=============================================================================
// Convert the integer in i to it's hesadecimal ASCII representation in buffer
// The integer is of length n
//=============================================================================
int intToHAsc(unsigned int i, char *buffer, int len)
{
	int count;

	for (count = 0; count < len; count++)
		buffer[count] = ' ';
	count = len - 1;
	do
	{
		if (i % 16 < 10)
			buffer[count] = '0' + i % 16;
		else
			buffer[count] = '0' + 7 + i % 16;
		i = i / 16;
		count--;
	} while (i > 0);
	return 0;
}

inline void outw(unsigned short port, unsigned short val)
{
	asm volatile( "outw %0, %1"
			: : "a"(val), "Nd"(port) );
}

inline unsigned short inw(unsigned short port)
{
	unsigned short ret;
	asm volatile( "inw %1, %0"
			: "=a"(ret) : "Nd"(port) );
	return ret;
}

inline void outl(unsigned short port, unsigned int val)
{
	asm volatile( "outl %0, %1"
			: : "a"(val), "Nd"(port) );
}

inline unsigned int inl(unsigned short port)
{
	unsigned int ret;
	asm volatile( "inl %1, %0"
			: "=a"(ret) : "Nd"(port) );
	return ret;
}

