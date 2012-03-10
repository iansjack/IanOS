#ifndef LIB_H
#define LIB_H

#include "kstructs.h"
#include "filesystem.h"

char getchar();
void send(struct Message * msg);
long GetDirectoryEntry(int n, struct DirEntry * entry);
int printf(unsigned char *s,...);
void ConsoleClrScr();
int intToAsc(int i, char *buffer, int len);
int intToHAsc(int i, char *buffer, int len);
unsigned char *strchr(unsigned char *string, unsigned char c);

#endif