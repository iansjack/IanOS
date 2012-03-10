//==================================================================
// Finds first occurrence of c in string.
// Returns pointer to that occurrence or 0 if not found
//==================================================================
unsigned char *strchr(unsigned char *string, unsigned char c)
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
long strlen(unsigned char *string)
{
	int i = 0;
	while (string[i++]);
	return i - 1;
}

//===============================================================
// Compares s1 and s2 up to length characters.
// Returns 0 if they are equal, non-zero otherwise.
//===============================================================
long strncmp(unsigned char * s1, unsigned char * s2, long length)
{
   long count;
   short done = 0;

   for (count = 1; count < length; count++)
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
unsigned char * strcpy(unsigned char *destination, unsigned char *source)
{
	unsigned char * retval = destination;
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
long strcmp(unsigned char *s1, unsigned char*s2)
{
 	long retval = 1;
	int count = 0;
	while (s1[count] == s2[count])
	{
		if (s1[count++] == 0) retval = 0;
	}
	return retval;
}

//===========================================================
// Concatenates s2 to the end of s1.
// Returns s1
//===========================================================
unsigned char *strcat(unsigned char *s1, unsigned char *s2)
{
	int n = strlen(s1);
	strcpy(s1 + n, s2);
	return s1;
}

