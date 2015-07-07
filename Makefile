include Flags.mak

OBJS = startup.o os.o mem32.o ptab32.o hwsetup.o gates.o pci.o messages.o memory.o pagetab.o keyboard.o \
		console.o vga.o filesystem.o block.o syscalls.o newtask.o tasking.o messaging.o interrupts.o \
		ide.o kernlib.o tasklist.o btree.o clock.o tas1.o scalls.o e1000.o net.o

all: IanOS

IanOS: IanOS.o IanOS.bin
	mount /home/ian/mnt
	cd library; make all; make install; cd ..
	cd netlib; make all; make install; cd ..
	cd tasks; make all; make install; cd ..
	umount /home/ian/mnt
	make -f Makefile2 myos.iso

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c
	$(CC) -MM $(CFLAGS) $*.c > $*.d

IanOS.o: $(OBJS) $(CROSS)/lib/libc.a
	$(LD) -Tlink2.ld $(OBJS) $(CROSS)/lib/libc.a -oIanOS.o
	
IanOS.bin: $(OBJS) $(CROSS)/lib/libc.a
	$(LD) -s --print-map -Tlink.ld $(OBJS) $(CROSS)/lib/libc.a -oIanOS.bin>linkmap 
	
startup.o: startup.s $(INC)/memory.inc

hwsetup.o: hwsetup.s hwhelp.s

syscalls.o: $(INC)/memory.inc

os.o: os.s $(INC)/memory.inc $(INC)/syscalls.inc macros.s $(INC)/kstructs.inc

vga.o: vga.s

tasking.o: tasking.s $(INC)/memory.inc $(INC)/kstructs.inc

interrupts.o: interrupts.s macros.s $(INC)/memory.h $(INC)/kstructs.h

ide.o: ide.s

mem32.o: mem32.c $(INC)/memory.h
	$(CC) -m32 -D CODE_32 $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -S mem32.c
	cat code32.s mem32.s >tmem32.s
	$(AS) tmem32.s -o mem32.o
	rm tmem32.s mem32.s

ptab32.o: ptab32.c $(INC)/memory.h
	$(CC) -m32 -D CODE_32 -fno-stack-protector -ffixed-r15 -g -I $(INC) -S ptab32.c
	cat code32.s ptab32.s >tptab32.s
	$(AS) tptab32.s -o ptab32.o
	rm tptab32.s ptab32.s

-include $(OBJS:.o=.d)

clean:
	rm -f linkmap *.o *.d *.bin *~ *.iso
	cd library; make clean; cd ..
	cd netlib; make clean; cd ..
	cd tasks; make clean

