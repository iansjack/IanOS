include ~/IanOS/gmsl
INC = ~/IanOS/include
CC=x86_64-elf-gcc
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -fno-dwarf2-cfi-asm -mno-red-zone -g \
		-I $(INC) -I /usr/local/cross/x86_64-elf/include