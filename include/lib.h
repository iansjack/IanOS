#ifndef LIB_H
#define LIB_H

#include <kstructs.h>
#include <filesystem.h>
#include <syscalls.h>

char getchar();
void send(struct Message *msg);
long GetDirectoryEntry(int n, struct DirEntry *entry);
int printf(unsigned char *s, ...);
int intToAsc(int i, char *buffer, int len);
int intToHAsc(int i, char *buffer, int len);
unsigned char *strchr(unsigned char *string, unsigned char c);
unsigned char *strcpy(unsigned char *dest, unsigned char *source);
unsigned char *strcat(unsigned char *s1, unsigned char *s2);

#endif
