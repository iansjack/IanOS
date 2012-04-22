include ../Flags.mak

LIBS = ../library/liblib.a ../library/libsyscalls.a 
TASKS = TASK1 TASK2 TASK3 TASK4 PWD LS CAT EDIT

all: inittask.o $(call lc, $(TASKS)) $(TASKS)

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c -o $@
	objcopy --remove-section .eh_frame $@

% : %.o
	ld -Tlink.ld $(call lc,$@).o $(LIBS) -o $@
	mcopy -o $@ c:/BIN
	
TASK1: task1.o inittask.o $(LIBS)
TASK2: task2.o inittask.o $(LIBS)
TASK3: task3.o inittask.o $(LIBS)
TASK4: task4.o inittask.o $(LIBS)
PWD: pwd.o inittask.o $(LIBS)
LS: ls.o inittask.o $(LIBS)
CAT: cat.o inittask.o $(LIBS)
EDIT: edit.o inittask.o $(LIBS)

clean:
	rm -f *.o $(TASKS) *~
