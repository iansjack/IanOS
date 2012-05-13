include ../Flags.mak
CC=x86_64-elf-gcc
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -fno-dwarf2-cfi-asm -mno-red-zone -g -I $(INC) -I /usr/local/cross/x86_64-elf/include -Xlinker -print-map >linkmap -Tlink.ld
TASKS = task1 task2 task3 task4 pwd ls cat edit sh

all: $(TASKS)

clean:
	rm -f *.o *.d $(TASKS) *~
