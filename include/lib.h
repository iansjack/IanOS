#ifndef LIB_H
#define LIB_H

#include <kstructs.h>
#include <filesystem.h>
#include <syscalls.h>

char getchar();
void send(struct Message *msg);
int printf(char *s, ...);
int intToAsc(unsigned int i, char *buffer, int len);
int intToHAsc(unsigned int i, char *buffer, int len);
char *strchr(char *string, char c);
char *strcpy(char *dest, char *source);
char *strcat(char *s1, char *s2);

#endif
