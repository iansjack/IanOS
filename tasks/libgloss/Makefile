INC = ~/IanOS/include
CC = x86_64-elf-gcc
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
		-I /usr/local/cross/x86_64-elf/include -I $(INC)
OBJS = _exit.o access.c alloc_page.o chdir.o chmod.o close.o creat.o execve.o fcntl.o fork.o \
		fstat.o getcwd.o \
		getpid.o gettimeofday.o getwd.o isatty.o kill.o lseek.o lstat.o mkdir.o open.o pathconf.o read.o \
		sbrk.o stat.o \
		sys_fstat.o sys_open.o umask.o unlink.o waitpid.o write.o

all: $(OBJS)

install: $(OBJS)	
	ar -r /usr/local/cross/x86_64-elf/lib/libc.a *.o
	
clean:
	rm -f *.o
