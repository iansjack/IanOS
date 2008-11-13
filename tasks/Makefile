CFLAGS = -fpack-struct -ffixed-r15 -O -I ..
LIBS = ../library/libsyscalls.a ../library/liblib.a
TASKS = TASK1.BIN TASK2.BIN TASK3.BIN

all: $(TASKS) 

inittask.o: inittask.s
	as inittask.s -o inittask.o

TASK1.BIN: task1.o inittask.o
	ld -Tlink.ld task1.o $(LIBS) -oTASK1.BIN
	mcopy -o TASK1.BIN c: 

task1.o: task1.c

TASK2.BIN: task2.o inittask.o
	ld -Tlink.ld task2.o $(LIBS) -oTASK2.BIN
	mcopy -o TASK2.BIN c: 

task2.o: task2.c

TASK3.BIN: task3.o inittask.o
	ld -Tlink.ld task3.o $(LIBS) -oTASK3.BIN
	mcopy -o TASK3.BIN c: 

task3.o: task3.c

clean:
	rm -f *.BIN *.o

