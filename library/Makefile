include ../Flags.mak

CFLAGS:=$(CFLAGS) -O -I ..
LIBS = $(INC)/library/liblib.a $(INC)/library/libsyscalls.a 
LIBOBJS = writeconsolechar.o writeconsolestring.o consoleclrscr.o \
	getchar.o createfile.o deletefile.o readfile.o writefile.o getfileinfo.o \
	getfspid.o getdirectoryentry.o getdirectory.o convert.o
SYSCALLSOBJS = syscalls.o allocmem.o allocsharedmem.o deallocmem.o \
	killtask.o getticks.o sleep.o

all: liblib.a libsyscalls.a

liblib.a: $(LIBOBJS)
	ar -r liblib.a $(LIBOBJS)

libsyscalls.a: $(SYSCALLSOBJS)
	ar -r libsyscalls.a $(SYSCALLSOBJS)

clean:
	rm -f *.a *.o *~

writeconsolechar.o: writeconsolechar.c $(INC)/console.h $(INC)/kstructs.h $(INC)/memory.h $(INC)/syscalls.h

writeconsolestring.o: writeconsolestring.c $(INC)/console.h $(INC)/kstructs.h $(INC)/memory.h $(INC)/syscalls.h

consoleclrscr.o: consoleclrscr.c $(INC)/console.h $(INC)/kstructs.h $(INC)/memory.h $(INC)/syscalls.h

createfile.o: createfile.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

openfile.o: openfile.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

closefile.o: closefile.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

deletefile.o: deletefile.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

readfile.o: readfile.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

writefile.o: writefile.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

getfileinfo.o: getfileinfo.c $(INC)/kstructs.h $(INC)/filesystem.h $(INC)/syscalls.h

getfspid.o: getfspid.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

getdirectoryentry.o: getdirectoryentry.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

getdirectory.o: getdirectory.c $(INC)/kstructs.h $(INC)/memory.h $(INC)/filesystem.h $(INC)/syscalls.h

syscalls.o: syscalls.s $(INC)/syscalls.inc

allocmem.o: allocmem.s $(INC)/syscalls.inc

deallocmem.o: deallocmem.s $(INC)/syscalls.inc

allocsharedmem.o: allocsharedmem.s $(INC)/syscalls.inc

killtask.o: killtask.s $(INC)/syscalls.inc

getcommandline.o: getcommandline.s $(INC)/syscalls.inc

getticks.o: getticks.s $(INC)/syscalls.inc

sleep.o: sleep.s $(INC)/syscalls.inc

convert.o: convert.c
