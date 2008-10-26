CFLAGS = -fpack-struct -O

OBJS = 	startup.o memory32.o pagetab32.o hwsetup.o os.o gates.o messages.o memory.o keyboard.o \
		fat.o syscalls.o newtask.o tasking.o messaging.o interrupts.o kernlib.o tas1.o

all: bootdisk
	cd library; make all
	cd tasks; make all

IanOS.bin: $(OBJS)
	ld --print-map -Tlink.ld $(OBJS) library/liblib.a library/libsyscalls.a -oIanOS.bin>linkmap 

bootdisk: bootsect.bin 32sect IanOS.bin
	cat bootsect.bin 32sect IanOS.bin >IanOS.fdd

bootsect.bin: boot.o
	ld -Tbootlink.ld boot.o -obootsect.bin

boot.o: boot.s
	as boot.s -o boot.o

startup.o: startup.s
	as  startup.s -o startup.o

memory32.o: memory32.s memory.h
	as  memory32.s -o memory32.o

pagetab32.o: pagetab32.s
	as  pagetab32.s -o pagetab32.o

hwsetup.o: hwsetup.s hwhelp.s
	as  hwsetup.s -o hwsetup.o

os.o: os.s memory.h syscalls.h macros.s kstructs.h

gates.o: gates.c cmemory.h

messages.o: messages.c

memory.o: memory.c cmemory.h ckstructs.h

keyboard.o: keyboard.c cmemory.h ckstructs.h

fat.o: fat.c cmemory.h ckstructs.h

syscalls.o: syscalls.s memory.h kstructs.h

newtask.o: newtask.c cmemory.h ckstructs.h

tasking.o: tasking.s memory.h kstructs.h

messaging.o: messaging.c cmemory.h ckstructs.h

interrupts.o: interrupts.s macros.s memory.h kstructs.h

kernlib.o: kernlib.c

tas1.o: tas1.c cmemory.h library/syscalls.h

clean:
	rm -f a.out linkmap $(OBJS) *.bin boot.o IanOS.fdd
	cd library; make clean
	cd tasks; make clean

