CFLAGS = -fpack-struct -ffixed-r15 -g

OBJS = 	startup.o memory32.o pagetab32.o hwsetup.o os.o gates.o messages.o memory.o keyboard.o \
		console.o filesystem.o syscalls.o newtask.o tasking.o messaging.o interrupts.o \
		ide.o kernlib.o tas1.o

all: bootdisk
	cd library; make all; cd ..
	cd tasks; make all; cd ..

library/liblib.a:
	cd library; make all; cd ..

library/libsyscalls.a:
	cd library; make all; cd ..

IanOS.o: $(OBJS) library/liblib.a library/libsyscalls.a
	ld -Tlink2.ld $(OBJS) library/liblib.a library/libsyscalls.a -oIanOS.o>linkmap 

IanOS.bin: $(OBJS) library/liblib.a library/libsyscalls.a
	ld --print-map -Tlink.ld $(OBJS) library/liblib.a library/libsyscalls.a -oIanOS.bin>linkmap 

bootdisk: bootsect.bin 32sect IanOS.bin
	cat bootsect.bin 32sect IanOS.bin floppy >IanOS.fdd

bootsect.bin: boot.o
	ld -Tbootlink.ld boot.o -obootsect.bin

boot.o: boot.s memory.inc
	as boot.s -o boot.o

startup.o: startup.s
	as  startup.s -o startup.o

memory32.o: memory32.s memory.inc
	as  memory32.s -o memory32.o

pagetab32.o: pagetab32.s
	as  pagetab32.s -o pagetab32.o

hwsetup.o: hwsetup.s hwhelp.s
	as  hwsetup.s -o hwsetup.o

os.o: os.s memory.inc syscalls.inc macros.s kstructs.inc

gates.o: gates.c memory.h

messages.o: messages.c

memory.o: memory.c memory.h kstructs.h

keyboard.o: keyboard.c memory.h kstructs.h

console.o: console.c memory.h kstructs.h console.h

filesystem.o: filesystem.c memory.h kstructs.h

syscalls.o: syscalls.s memory.inc kstructs.inc

newtask.o: newtask.c memory.h kstructs.h filesystem.h

tasking.o: tasking.s memory.inc kstructs.inc

messaging.o: messaging.c memory.h kstructs.h

interrupts.o: interrupts.s macros.s memory.h kstructs.h

ide.o: ide.s

kernlib.o: kernlib.c

tas1.o: tas1.c memory.h library/syscalls.h

clean:
	rm -f a.out linkmap $(OBJS) *.bin boot.o IanOS.fdd
	cd library; make clean
	cd tasks; make clean

