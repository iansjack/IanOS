include ../Flags.mak
INC = -I /home/ian/IanOS/include
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
		-I $(CROSS)/include $(INC) 
LDFLAGS = -Xlinker -N -Xlinker -Ttext=0x4000000
LDLIBS = ~/IanOS/netlib/libnet.a
TASKS = pwd ls cat edit sh rm cp test nettest netserver dhcp

all : $(TASKS)

install-%:	
	cp $* ~/mnt/bin/

install: $(TASKS)
	for task in $(TASKS); do \
		$(MAKE) install-$$task; \
	done

netserver: netserver.c ../netlib/libnet.a

clean:
	rm -f *.o *.d $(TASKS) *~
