include ../Flags.mak

CFLAGS:=$(CFLAGS) -O -I ..
LIBS = ../library/liblib.a ../library/libsyscalls.a 
LIBOBJS = writeconsolechar.o writeconsolestring.o consoleclrscr.o \
	getchar.o createfile.o deletefile.o openfile.o closefile.o readfile.o writefile.o \
	getfspid.o getdirectoryentry.o getdirectory.o convert.o
SYSCALLSOBJS = syscalls.o allocmem.o allocsharedmem.o deallocmem.o \
	createtask.o createktask.o createlptask.o killtask.o\
	getcommandline.o getticks.o sleep.o

all: liblib.a libsyscalls.a

liblib.a: $(LIBOBJS)
	ar -r liblib.a $(LIBOBJS)

libsyscalls.a: $(SYSCALLSOBJS)
	ar -r libsyscalls.a $(SYSCALLSOBJS)

clean:
	rm -f *.a *.o *~

writeconsolechar.o: writeconsolechar.c ../console.h ../kstructs.h ../memory.h syscalls.h

writeconsolestring.o: writeconsolestring.c ../console.h ../kstructs.h ../memory.h syscalls.h

consoleclrscr.o: consoleclrscr.c ../console.h ../kstructs.h ../memory.h syscalls.h

createfile.o: createfile.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

openfile.o: openfile.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

closefile.o: closefile.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

deletefile.o: deletefile.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

readfile.o: readfile.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

writefile.o: writefile.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

getfspid.o: getfspid.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

getdirectoryentry.o: getdirectoryentry.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

getdirectory.o: getdirectory.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

syscalls.o: syscalls.s ../syscalls.inc

allocmem.o: allocmem.s ../syscalls.inc

deallocmem.o: deallocmem.s ../syscalls.inc

allocsharedmem.o: allocsharedmem.s ../syscalls.inc

createktask.o: createktask.s ../syscalls.inc

createtask.o: createtask.s ../syscalls.inc

createlptask.o: createlptask.s ../syscalls.inc

killtask.o: killtask.s ../syscalls.inc

getcommandline.o: getcommandline.s ../syscalls.inc

getticks.o: getticks.s ../syscalls.inc

sleep.o: sleep.s ../syscalls.inc

convert.o: convert.c
