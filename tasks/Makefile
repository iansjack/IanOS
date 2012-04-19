include ../Flags.mak

LIBS = ../library/liblib.a ../library/libsyscalls.a 
TASKS = TASK1 TASK2 TASK3 TASK4 PWD LS CAT EDIT

all: $(TASKS) 

inittask.o: inittask.s
	as inittask.s -o inittask.o

inittask2.o: inittask.s
	as inittask2.s -o inittask2.o

TASK1: task1.o inittask.o $(LIBS)
	ld -Tlink.ld task1.o $(LIBS) -oTASK1
	mcopy -o TASK1 c:/BIN 

task1.o: task1.c

TASK2: task2.o inittask.o $(LIBS)
	ld -Tlink.ld task2.o $(LIBS) -oTASK2
	mcopy -o TASK2 c:/BIN 

task2.o: task2.c

TASK3: task3.o inittask.o $(LIBS)
	ld -Tlink.ld task3.o $(LIBS) -oTASK3
	mcopy -o TASK3 c:/BIN 

task3.o: task3.c

TASK4: task4.o inittask.o $(LIBS)
	ld -Tlink.ld task4.o $(LIBS) -oTASK4
	mcopy -o TASK4 c:/BIN 

task4.o: task4.c 

PWD: pwd.o inittask.o $(LIBS)
	ld -Tlink.ld pwd.o $(LIBS) -oPWD
	mcopy -o PWD c:/BIN 

task4.o: task4.c 

LS: ls.o inittask.o $(LIBS)
	ld -Tlink.ld ls.o $(LIBS) -oLS
	mcopy -o LS c:/BIN 

ls.o: ls.c

CAT: cat.o inittask.o $(LIBS)
	ld -Tlink.ld cat.o $(LIBS) -oCAT
	mcopy -o CAT c:/BIN 

cat.o: cat.c

EDIT: edit.o inittask.o $(LIBS)
	ld -Tlink.ld edit.o $(LIBS) -oEDIT
	mcopy -o EDIT c:/BIN
	
edit: edit.o inittask.o $(LIBS)
	ld -Tlink2.ld edit.o $(LIBS) -oedit
 

edit.o: edit.c

clean:
	rm -f TASK? *.o LS* CAT* PWD* EDIT* *~

