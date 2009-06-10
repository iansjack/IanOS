CFLAGS = -fpack-struct -ffixed-r15 -O -I ..
LIBS = ../library/liblib.a ../library/libsyscalls.a 
LIBOBJS = writestring.o getticks.o writedouble.o sendreceive.o writeconsolestring.o getchar.o file.o
SYSCALLSOBJS = syscalls.o

all: liblib.a libsyscalls.a

liblib.a: $(LIBOBJS)
	ar -r liblib.a $(LIBOBJS)

libsyscalls.a: $(SYSCALLSOBJS)
	ar -r libsyscalls.a $(SYSCALLSOBJS)

clean:
	rm -f *.a *.o

writeconsolestring.o: writeconsolestring.c ../console.h ../kstructs.h ../memory.h syscalls.h

file.o: file.c ../kstructs.h ../memory.h ../filesystem.h syscalls.h

syscalls.o: syscalls.s ../syscalls.inc