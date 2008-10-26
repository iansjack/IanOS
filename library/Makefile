LIBOBJS = clearscr.o writestring.o getticks.o writedouble.o sendreceive.o writechar.o send.o
SYSCALLSOBJS = sys_AllocMem.o sys_CreateTask.o sys_DeallocMem.o sys_KillTask.o sys_ReceiveMessage.o

all: liblib.a libsyscalls.a

liblib.a: $(LIBOBJS)
	ar -r liblib.a $(LIBOBJS)

libsyscalls.a: $(SYSCALLSOBJS)
	ar -r libsyscalls.a $(SYSCALLSOBJS)

clean:
	rm $(LIBOBJS) $(SYSCALLSOBJS)
