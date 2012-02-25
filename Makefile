include Flags.mak

OBJS = startup.o mem32.o ptab32.o hwsetup.o os.o gates.o messages.o memory.o pagetab.o keyboard.o \
		console.o filesystem.o syscalls.o newtask.o tasking.o messaging.o interrupts.o \
		ide.o kernlib.o  tasklist.o tas1.o

all: bootdisk IanOS.o
	cd library; make all; cd ..
	cd tasks; make all; cd ..

library/liblib.a:
	cd library; make all; cd ..

library/libsyscalls.a:
	cd library; make all; cd ..

IanOS.o: $(OBJS) library/liblib.a library/libsyscalls.a
	ld -Tlink2.ld $(OBJS) library/liblib.a library/libsyscalls.a -oIanOS.o 

IanOS.bin: $(OBJS) library/liblib.a library/libsyscalls.a
	ld --print-map -Tlink.ld $(OBJS) library/liblib.a library/libsyscalls.a -oIanOS.bin>linkmap 

bootdisk: bootsect.bin 32sect IanOS.bin
	cat bootsect.bin 32sect IanOS.bin floppy >I.fdd
	dd if=I.fdd of=IanOS.fdd count=2880
	rm I.fdd
	qemu-img convert IanOS.fdd -O raw IanOS.vfd

bootsect.bin: boot.o
	ld -Tbootlink.ld boot.o -obootsect.bin

boot.o: boot.s memory.inc
	as boot.s -o boot.o

startup.o: startup.s
	as  startup.s -o startup.o
	
starttask.o: starttask.s syscalls.inc

mem32.o: mem32.c	memory.h
	gcc -m32 -D CODE_32 $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -S mem32.c
	cat code32.s mem32.s >tmem32.s
	as tmem32.s -o mem32.o
	rm tmem32.s mem32.s

ptab32.o: ptab32.c memory.h
	gcc -m32 -D CODE_32 -fno-stack-protector -ffixed-r15 -g -I . -I .. -S ptab32.c
	cat code32.s ptab32.s >tptab32.s
	as tptab32.s -o ptab32.o
	rm tptab32.s ptab32.s
	
hwsetup.o: hwsetup.s hwhelp.s
	as  hwsetup.s -o hwsetup.o

os.o: os.s memory.inc syscalls.inc macros.s kstructs.inc

gates.o: gates.c memory.h

messages.o: messages.c

memory.o: memory.c memory.h kstructs.h tasklist.h

pagetab.o: pagetab.c memory.h pagetab.h kstructs.h

keyboard.o: keyboard.c memory.h kstructs.h

console.o: console.c memory.h kstructs.h console.h

filesystem.o: filesystem.c memory.h kstructs.h

syscalls.o: syscalls.s memory.inc kstructs.inc

newtask.o: newtask.c memory.h kstructs.h filesystem.h kernel.h fat.h tasklist.h

tasking.o: tasking.s memory.inc kstructs.inc

messaging.o: messaging.c memory.h kstructs.h

interrupts.o: interrupts.s macros.s memory.h kstructs.h

ide.o: ide.s

kernlib.o: kernlib.c

tasklist.o: tasklist.c tasklist.h memory.h

tas1.o: tas1.c memory.h library/syscalls.h

monitor.o: monitor.c memory.h kernel.h console.h

clean:
	rm -f linkmap $(OBJS) *.bin boot.o IanOS.fdd *~
	cd library; make clean
	cd tasks; make clean

