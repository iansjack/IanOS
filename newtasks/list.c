#include <stdio.h>

int main(int argc, char **argv)
{
  	char c;
  	puts ("Enter text. Include a dot ('.') in a sentence to exit:");
  	do {
    		c=getchar();
    		putchar (c);
  	} while (c != '.');
	FILE *file = fopen("TEST.TXT", "r");
	char *buffer = (char *)malloc(10);
	fread(buffer, 5, 1, file);
	printf("%d\n", file);
	printf("%c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3]);
	fclose(file);
  return 0;
}
