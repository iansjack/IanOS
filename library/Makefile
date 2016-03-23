include ../Flags.mak
INC = -I /home/ian/IanOS/include
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g -fPIC\
		-I $(CROSS)/include $(INC) 
OBJS = sys_queuepacket.o sys_receivepacket.o sys_getnetport.o sys_receivemessage.o \
		sys_allocmessageport.o sys_sendmessage.o sys_sendreceive.o \
		 _exit.o access.o alloc_page.o	alloc_shared_page.o chdir.o chmod.o close.o creat.o dummies.o execve.o \
		fcntl.o fork.o fstat.o getcwd.o getpid.o gettimeofday.o getwd.o isatty.o kill.o lseek.o lstat.o \
		mkdir.o open.o pathconf.o read.o  sbrk.o stat.o sys_close.o sys_execve.o sys_fstat.o sys_lstat.o \
		sys_nanosleep.o sys_open.o sys_setalarm.o sys_time.o sys_truncate.o sys_unlink.o sys_write.o \
		umask.o unlink.o waitpid.o write.o extras.o memory.o
				
NEWLIB_OBJS = newlib/lib_a-*.o

all: $(OBJS) crt0 libc.so

crt0: crt0.s
	$(AS) crt0.s -ocrt0.o
	
libc.so: $(OBJS)
	ld -shared -o libc.so $(NEWLIB_OBJS) $(OBJS)

install: $(OBJS) libc.so	
	cp crt0.o $(CROSS)/lib
	cp libc.so $(CROSS)/lib
	cp libc.so ~/mnt/lib
	strip -d ~/mnt/lib/*
	cp libc.so ../tasks
	cp libc.so /usr/local/cross/x86_64-elf/lib
	mv crt0.o crt0.old
	ar -r $(CROSS)/lib/libc.a $(OBJS)
	mv crt0.old crt0.o
	
clean:
	rm -f *.o *.so
