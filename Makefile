include Flags.mak

OBJS = startup.o os.o mem32.o ptab32.o hwsetup.o gates.o messages.o memory.o pagetab.o keyboard.o \
		console.o vga.o filesystem.o block.o syscalls.o newtask.o tasking.o messaging.o interrupts.o \
		ide.o kernlib.o tasklist.o btree.o clock.o tas1.o scalls.o

all: bootdisk IanOS.o
	cd library; make all; cd ..
	cd tasks; make all; make install; cd ..

library/liblib.a:
	cd library; make all; cd ..

library/libsyscalls.a:
	cd library; make all; cd ..

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c
	gcc -MM $(CFLAGS) $*.c > $*.d

IanOS.o: $(OBJS) library/liblib.a library/libsyscalls.a /usr/local/cross/x86_64-elf/lib/libc.a
#	ld -Tlink2.ld $(OBJS) library/liblib.a library/libsyscalls.a -oIanOS.o
	ld -Tlink2.ld $(OBJS) /usr/local/cross/x86_64-elf/lib/libc.a -oIanOS.o
	
IanOS.bin: $(OBJS) library/liblib.a library/libsyscalls.a
#	ld -s --print-map -Tlink.ld $(OBJS) library/liblib.a library/libsyscalls.a /usr/local/cross/x86_64-elf/lib/libc.a -oIanOS.bin>linkmap 
	ld -s --print-map -Tlink.ld $(OBJS) /usr/local/cross/x86_64-elf/lib/libc.a -oIanOS.bin>linkmap 
	
bootdisk: bootsect.bin 32sect IanOS.bin
	cat bootsect.bin 32sect IanOS.bin floppy >I.fdd
	dd if=I.fdd of=IanOS.fdd count=2880
	rm I.fdd
	qemu-img convert IanOS.fdd -O raw IanOS.vfd

bootsect.bin: boot.o
	ld -s -Tbootlink.ld boot.o -obootsect.bin

boot.o: boot.s $(INC)/memory.inc

startup.o: startup.s $(INC)/memory.inc

hwsetup.o: hwsetup.s hwhelp.s

syscalls.o: $(INC)/memory.inc

os.o: os.s $(INC)/memory.inc $(INC)/syscalls.inc macros.s $(INC)/kstructs.inc

vga.o: vga.s

tasking.o: tasking.s $(INC)/memory.inc $(INC)/kstructs.inc

interrupts.o: interrupts.s macros.s $(INC)/memory.h $(INC)/kstructs.h

ide.o: ide.s

mem32.o: mem32.c $(INC)/memory.h
	gcc -m32 -D CODE_32 $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -S mem32.c
	cat code32.s mem32.s >tmem32.s
	as tmem32.s -o mem32.o
	rm tmem32.s mem32.s

ptab32.o: ptab32.c $(INC)/memory.h
	gcc -m32 -D CODE_32 -fno-stack-protector -ffixed-r15 -g -I $(INC) -S ptab32.c
	cat code32.s ptab32.s >tptab32.s
	as tptab32.s -o ptab32.o
	rm tptab32.s ptab32.s

-include $(OBJS:.o=.d)

clean:
	rm -f linkmap *.o *.d *.bin *~
	cd library; make clean
	cd tasks; make clean
