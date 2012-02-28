include ../Flags.mak

LIBS = ../library/liblib.a ../library/libsyscalls.a 
TASKS = TASK1 LS CAT

all: $(TASKS) 

inittask.o: inittask.s
	as inittask.s -o inittask.o

TASK1: task1.o inittask.o $(LIBS)
	ld --print-map -Tlink.ld task1.o $(LIBS) -oTASK1 >task1.map
	mcopy -o TASK1 c: 

task1.o: task1.c

TASK2: task2.o inittask.o $(LIBS)
	ld -Tlink.ld task2.o $(LIBS) -oTASK2
	mcopy -o TASK2 c: 

task2.o: task2.c

TASK3: task3.o inittask.o $(LIBS)
	ld -Tlink.ld task3.o $(LIBS) -oTASK3
	mcopy -o TASK3 c: 

task3.o: task3.c

TASK4: task4.o inittask.o $(LIBS)
	ld -Tlink.ld task4.o $(LIBS) -oTASK4
	mcopy -o TASK4 c: 

task4.o: task4.c 

LS: ls.o inittask.o $(LIBS)
	ld -Tlink.ld ls.o $(LIBS) -oLS
	mcopy -o LS c: 

ls.o: ls.c

CAT: cat.o inittask.o $(LIBS)
	ld -Tlink.ld cat.o $(LIBS) -oCAT
	mcopy -o CAT c: 

cat.o: cat.c

clean:
	rm -f TASK? *.o LS* *~

