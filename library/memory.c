

memset(char *address, char value, long len)
{
	long i;
	for (i = 0; i < len; i++)
		address[i] = value;
}

memcpy(char *dest, char *source, long len)
{
	long i;
	for (i = 0; i < len; i++)
		dest[i] = source[i];
}
