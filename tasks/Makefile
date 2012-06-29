#include ../Flags.mak
INC = -I ~/IanOS/include
CC = x86_64-elf-gcc
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
		-I /usr/local/cross/x86_64-elf/include $(INC)
LDFLAGS = -Xlinker --print-map >linkmap -Tlink.ld
TASKS = pwd ls cat edit sh rm task4

all: $(TASKS)

install-%:	
	cp $* ~/mnt/bin/

install: $(TASKS)
	for task in $(TASKS); do \
		$(MAKE) install-$$task; \
	done
		
clean:
	rm -f *.o *.d $(TASKS) *~
