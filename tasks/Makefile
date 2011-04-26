CFLAGS = -fpack-struct -ffixed-r15 -fno-stack-protector -O -I ..
LIBS = ../library/liblib.a ../library/libsyscalls.a 
TASKS = TASK1 TASK2 TASK3 TASK4 LS

all: $(TASKS) 

inittask.o: inittask.s
	as inittask.s -o inittask.o

TASK1: task1.o inittask.o
	ld -Tlink.ld task1.o $(LIBS) -oTASK1
	mcopy -o TASK1 c: 

task1.o: task1.c

TASK2: task2.o inittask.o
	ld -Tlink.ld task2.o $(LIBS) -oTASK2
	mcopy -o TASK2 c: 

task2.o: task2.c

TASK3: task3.o inittask.o
	ld -Tlink.ld task3.o $(LIBS) -oTASK3
	mcopy -o TASK3 c: 

task3.o: task3.c

TASK4: task4.o inittask.o
	ld -Tlink.ld task4.o $(LIBS) -oTASK4
	mcopy -o TASK4 c: 

task4.o: task4.c

LS: ls.o inittask.o
	ld -Tlink.ld ls.o $(LIBS) -oLS
	mcopy -o LS c: 

ls.o: ls.c

clean:
	rm -f TASK? *.o LS*

