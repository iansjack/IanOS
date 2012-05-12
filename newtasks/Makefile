include ../Flags.mak
CC=x86_64-elf-gcc
CFLAGS+=-Tlink.ld

TASKS = task1 task2 task3 task4 pwd ls cat edit

all: $(TASKS)

clean:
	rm -f *.o *.d $(TASKS) *~
