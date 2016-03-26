include Flags.mak

# *** Note that in this list of object files scalls.o must immediately follow tas1.o!!! ***

OBJS = os.o gates.o pci.o messages.o memory.o pagetab.o keyboard.o \
		console.o vga.o filesystem.o block.o syscalls.o newtask.o tasking.o elffunctions.o messaging.o interrupts.o \
		ide.o kernlib.o tasklist.o btree.o clock.o timer.o tas1.o scalls.o e1000.o net.o lapic.o mpentry.o mp.o

all: IanOS

IanOS: IanOS.o IanOS.bin
	mount /home/ian/mnt
	cd library; make all; make install; cd ..
	cd netlib; make all; make install; cd ..
	cd tasks; make all; make install; cd ..
	umount /home/ian/mnt
	cd boot; make iso; cd ..

%.o : %.c
	$(CC) $(CFLAGS) -c $*.c
	$(CC) -MM $(CFLAGS) $*.c > $*.d

IanOS.o: $(OBJS) $(CROSS)/lib/libc.a
	$(LD) -Tlink2.ld $(OBJS) $(CROSS)/lib/libc.a -oIanOS.o
	
IanOS.bin: $(OBJS) $(CROSS)/lib/libc.a
	$(LD) -s --print-map -Tlink.ld $(OBJS) $(CROSS)/lib/libc.a -oIanOS.bin>linkmap 
	
syscalls.o: $(INC)/memory.inc

os.o: os.s $(INC)/memory.inc $(INC)/syscalls.inc macros.s $(INC)/kstructs.inc

vga.o: vga.s

tasking.o: tasking.s $(INC)/memory.inc $(INC)/kstructs.inc

interrupts.o: interrupts.s macros.s $(INC)/memory.h $(INC)/kstructs.h

ide.o: ide.s

mpentry.o: mpentry.s

-include $(OBJS:.o=.d)

timer.o: timer.c $(INC)/timer.h

$(INC)/kstructs.inc: $(INC)/kstructs.h
	sh $(INC)/convert.sh > $(INC)/kstructs.inc

clean:
	rm -f linkmap *.o *.d *.bin *~ *.iso
	cd library; make clean; cd ..
	cd netlib; make clean; cd ..
	cd tasks; make clean
	cd boot; make clean

