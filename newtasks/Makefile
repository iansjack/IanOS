include ../Flags.mak
CFLAGS = -fno-stack-protector -fno-builtin -ffixed-r15 -mno-red-zone -g \
 		-I $(INC) -I /usr/local/cross/x86_64-elf/include -Xlinker -print-map >linkmap -Tlink.ld
TASKS = pwd ls cat edit sh

all: $(TASKS)

install-%:	
	mcopy -o $* C:/BIN/$(call uc, $*)

install: $(TASKS)
	for task in $(TASKS); do \
		$(MAKE) install-$$task; \
	done
		
clean:
	rm -f *.o *.d $(TASKS) *~
