include ../Flags.mak
INC = -I /home/ian/IanOS/include
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
		-I $(CROSS)/include $(INC) 
OBJS = _exit.o access.o alloc_page.o chdir.o chmod.o close.o creat.o execve.o fcntl.o fork.o \
		fstat.o getcwd.o \
		getpid.o gettimeofday.o getwd.o isatty.o kill.o lseek.o lstat.o mkdir.o open.o pathconf.o read.o \
		sbrk.o stat.o sys_close.o sys_execve.o sys_lstat.o \
		sys_fstat.o sys_open.o sys_time.o sys_truncate.o sys_unlink.o sys_write.o umask.o unlink.o \
		waitpid.o write.o dummies.o

all: $(OBJS) crt0

crt0: crt0.s
	$(AS) crt0.s -ocrt0.o

install: $(OBJS)	
	cp crt0.o $(CROSS)/lib
	mv crt0.o crt0.old
	ar -r $(CROSS)/lib/libc.a *.o
	mv crt0.old crt0.o
	
clean:
	rm -f *.o
