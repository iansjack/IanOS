#include "syscalls.h"
#include <stdarg.h>

int printf(unsigned char *s, ...)
{
	va_list ap;
	va_start(ap, s);

	unsigned char sprocessed[256];
	int i = 0;
	int j = 0;
	int k = 0;
	while (s[i]) {
		if (s[i] != '%') {
			sprocessed[j] = s[i];
			i++;
			j++;
		} else {
			i++;
			switch (s[i]) {
			case 'c':
				;
				int c = va_arg(ap, int);
				sprocessed[j++] = c;
				break;
			case 's':
				;
				unsigned char *s1 = va_arg(ap, unsigned char *);
				while (s1[k])
					sprocessed[j++] = s1[k++];
				break;
			case 'd':
				;
				char buffer[8];
				int number = va_arg(ap, int);
				intToAsc(number, buffer, 8);
				for (k = 0; k < 8; k++)
					if (buffer[k] != ' ')
						sprocessed[j++] = buffer[k];
				break;
			case 'x':
				;
				number = va_arg(ap, int);
				intToHAsc(number, buffer, 8);
				sprocessed[j++] = '0';
				sprocessed[j++] = 'x';
				for (k = 0; k < 8; k++)
					sprocessed[j++] = buffer[k];
				break;
			default:
				break;
			}
			i++;
		}
	}
	va_end(ap);
	write(STDOUT, sprocessed, j);
	return 0;
}
