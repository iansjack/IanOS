#include <syscalls.h>
#include <stdarg.h>

int printf(unsigned char *s, ...)
{
	va_list ap;
	va_start(ap, s);

	unsigned char sprocessed[256];
	int i = 0;
	int j = 0;
	int k = 0;
	short minwidth = 0;
	short zeropadding = 0;
	short indicator = 0;

	while (s[i])
	{
		minwidth = 0;
		zeropadding = 0;
		indicator= 0;
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
			case 'c':
				while (minwidth-- > 1)
					sprocessed[j++] = ' ';
				int c = va_arg(ap, int);
				sprocessed[j++] = c;
				break;
			case 's':
				;
				unsigned char *s1 = va_arg(ap, unsigned char *);
				while (minwidth-- > strlen(s1))
					sprocessed[j++] = ' ';
				while (s1[k])
					sprocessed[j++] = s1[k++];
				break;
			case 'd':
				;
				char buffer[20];
				int number = va_arg(ap, int);
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
	}va_end(ap);
	write(STDOUT, sprocessed, j);
	return 0;
}
