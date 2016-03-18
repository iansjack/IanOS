PREFIX=~/opt/cross

TARGET=i586-elf

CC=$(PREFIX)/bin/$(TARGET)-gcc -I../include -fno-builtin -g

AS=$(PREFIX)/bin/$(TARGET)-as

LD=$(PREFIX)/bin/$(TARGET)-ld

OBJS=boot.o ptab32.o mem32.o hwsetup.o

iso: myos.iso

ptab32.o: ptab32.c ../include/memory.h ../include/pagetab32.h

mem32.o: mem32.c ../include/kernel.h ../include/mp.h

hwsetup.o: hwsetup.s hwhelp.s
	$(AS) hwsetup.s -o hwsetup.o

boot.o: boot.s startup.s
	$(AS) boot.s -o boot.o

myos.bin: $(OBJS)
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib $(OBJS) -lgcc

myos.iso: myos.bin ../IanOS.bin
	cp myos.bin ../isodir/boot/myos.bin
	cp ../IanOS.bin ../isodir/boot
	grub2-mkrescue -o ../myos.iso ../isodir
	
clean:
	rm -f *.o
	rm -f *.bin
	rm -f ../myos.iso
	

