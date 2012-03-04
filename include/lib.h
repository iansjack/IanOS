#ifndef LIB_H
#define LIB_H

#include "kstructs.h"
#include "filesystem.h"

char getchar();
void send(struct Message * msg);
long GetDirectoryEntry(int n, struct DirEntry * entry);
void printf(char *s);

#endif
