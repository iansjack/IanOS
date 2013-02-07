#include ../Flags.mak
CROSS = /usr/local/cross/x86_64-elf/lib/
INC = -I /home/ian/IanOS/include
CC = x86_64-elf-gcc
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
		-I /usr/local/cross/x86_64-elf/include $(INC)
#LDFLAGS = -Xlinker --print-map >linkmap -Tlink.ld
LDFLAGS = -N -Ttext 0x4000000
TASKS = pwd ls cat edit sh rm task4

all:
	for task in $(TASKS); do \
		$(MAKE) $$task.o; \
		$(MAKE) $$task; \
	done

install-%:	
	cp $* ~/mnt/bin/

install: $(TASKS)
	for task in $(TASKS); do \
		$(MAKE) install-$$task; \
	done
	
%.o : %.c
	$(CC) -c $(CFLAGS) -I $(INC) $*.c -o $*.o
	
% : %.o
	ld $(LDFLAGS) $(CROSS)crt0.o $*.o $(CROSS)libc.a -o$*
		
clean:
	rm -f *.o *.d $(TASKS) *~
