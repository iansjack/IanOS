include ../Flags.mak
INC = -I /home/ian/IanOS/include
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
		-I $(CROSS)/include $(INC) 
OBJS = arp.o helpers.o dhcp.o tcp.o

all: $(OBJS)

install: $(OBJS)	
	ar -r libnet.a *.o
	
clean:
	rm -f *.o
