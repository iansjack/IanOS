include ../Flags.mak

LIBS = ../library/liblib.a ../library/libsyscalls.a 
TASKS = task1 task2 task3 task4 pwd ls cat edit

all: inittask.o $(call lc, $(TASKS)) $(TASKS)

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o
#	objcopy --remove-section .eh_frame $*.o

% : %.o
	ld -Tlink.ld $(call lc,$@).o $(LIBS) -o $@
	mcopy -o $@ c:/BIN/$(call uc, $@)
	
task1: task1.o inittask.o $(LIBS)
task2: task2.o inittask.o $(LIBS)
task3: task3.o inittask.o $(LIBS)
task4: task4.o inittask.o $(LIBS)
pwd: pwd.o inittask.o $(LIBS)
ls: ls.o inittask.o $(LIBS)
cat: cat.o inittask.o $(LIBS)
edit: edit.o inittask.o $(LIBS)

clean:
	rm -f *.o *.d $(TASKS) *~
