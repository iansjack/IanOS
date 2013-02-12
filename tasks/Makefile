include ../Flags.mak
CROSS = /usr/local/cross/x86_64-elf/lib/
INC = -I /home/ian/IanOS/include
#CC = x86_64-elf-gcc
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
		-I /usr/local/cross/x86_64-elf/include $(INC)
LDFLAGS = -Xlinker -N -Xlinker -Ttext=0x4000000
TASKS = pwd ls cat edit sh rm

all : $(TASKS)

install-%:	
	cp $* ~/mnt/bin/

install: $(TASKS)
	for task in $(TASKS); do \
		$(MAKE) install-$$task; \
	done
	
clean:
	rm -f *.o *.d $(TASKS) *~
