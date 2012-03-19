/***************************************************************************
 *            convert.c
 *
 *  Sun Jul  5 08:51:53 2009
 *  Copyright  2009  ian
 *  <ian@Fedora10.ijack.org.uk>
 ****************************************************************************/

int intToAsc(int i, char *buffer, int len)
{
	int count;

	for (count = 0; count < len; count++)
		buffer[count] = ' ';
	count = len - 1;
	do {
		buffer[count] = '0' + i % 10;
		i = i / 10;
		count--;
	}
	while (i > 0);
	return 0;
}

int intToHAsc(int i, char *buffer, int len)
{
	int count;

	for (count = 0; count < len; count++)
		buffer[count] = '0';
	count = len - 1;
	do {
		int mod;
		if (i % 16 < 10)
			buffer[count] = '0' + i % 16;
		else
			buffer[count] = '0' + 7 + i % 16;
		i = i / 16;
		count--;
	}
	while (i > 0);
	return 0;
}
