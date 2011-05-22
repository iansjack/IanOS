#ifndef LIB_H
#define LIB_H

#include "kstructs.h"

char getchar();
void send(struct Message * msg);
void writeconsolechar(char c);
void writeconsolestring(char *s);
void consoleclrscr();
struct FCB *CreateFile(char *s);
struct FCB *OpenFile(char *s);
struct FCB *CloseFile(struct FCB *fHandle);
long ReadFile(struct FCB *fHandle, char *buffer, long noBytes);
long WriteFile(struct FCB *fHandle, char *buffer, long noBytes);
struct FCB *DeleteFile(struct FCB *fHandle);
long GetDirectoryEntry(int n, struct DirEntry * entry);

#endif
