CROSS = /usr/local/cross/x86_64-elf
include /home/ian/IanOS/gmsl
INC = /home/ian/IanOS/include
CCPATH=/usr/local/cross/bin
CCPREFIX=x86_64-elf-
CC=${CCPATH}/${CCPREFIX}gcc
AS=${CCPATH}/${CCPREFIX}as
LD=${CCPATH}/${CCPREFIX}ld
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -fno-dwarf2-cfi-asm -mno-red-zone -g \
		-I $(INC) -I $(CROSS)/include -I /usr/include