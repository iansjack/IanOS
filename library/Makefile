include ../Flags.mak

CFLAGS:=$(CFLAGS) -O -I ..
LIBS = $(INC)/library/liblib.a $(INC)/library/libsyscalls.a 
LIBOBJS = printf.o string.o \
	getchar.o convert.o
SYSCALLSOBJS = syscalls.o allocmem.o allocsharedmem.o deallocmem.o \
	killtask.o getticks.o

all: liblib.a libsyscalls.a

liblib.a: $(LIBOBJS)
	ar -r liblib.a $(LIBOBJS)

libsyscalls.a: $(SYSCALLSOBJS)
	ar -r libsyscalls.a $(SYSCALLSOBJS)

clean:
	rm -f *.a *.o *~

printf.o: printf.c $(INC)/console.h $(INC)/kstructs.h $(INC)/memory.h $(INC)/syscalls.h

syscalls.o: syscalls.s $(INC)/syscalls.inc

string.o: string.c 

allocmem.o: allocmem.s $(INC)/syscalls.inc

deallocmem.o: deallocmem.s $(INC)/syscalls.inc

allocsharedmem.o: allocsharedmem.s $(INC)/syscalls.inc

killtask.o: killtask.s $(INC)/syscalls.inc

getcommandline.o: getcommandline.s $(INC)/syscalls.inc

getticks.o: getticks.s $(INC)/syscalls.inc

#sleep.o: sleep.s $(INC)/syscalls.inc

convert.o: convert.c
