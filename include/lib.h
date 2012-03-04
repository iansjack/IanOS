#ifndef LIB_H
#define LIB_H

#include "kstructs.h"
#include "filesystem.h"

char getchar();
void send(struct Message * msg);
void consoleclrscr();
struct FCB *CloseFile(struct FCB *fHandle);
struct FCB *DeleteFile(struct FCB *fHandle);
long GetDirectoryEntry(int n, struct DirEntry * entry);
void printf(char *s);

#endif
