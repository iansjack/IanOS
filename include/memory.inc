#=============
# Memory Map
#=============
GDT	     		= 0x0000000000000000
IDT				= 0x0000000000000800
OSCode	     	= 0x0000000000001000
OSData	     	= 0x0000000000010000
OSHeap			= 0x0000000000012000
TempUserCode 	= 0x00000000000FD000
TempUserData 	= 0x00000000000FE000
TempUStack   	= 0x00000000000FF000
PageMap      	= 0x0000000000100000
UserCode     	= 0x0000000004000000
UserData     	= 0x0000000010000000
UserStack		= 0x00000000ffffe000
KernelStack    	= 0x00000000fffff000

PageSize	= 0x1000
Vaddr 		= 39	# The bit position to set to add 0x8000000000

OsCodeSeg 	= 0x8
OsDataSeg 	= 0x10
code64	  	= 0x18
data64		= 0x20
udata64		= 0x28
user64		= 0x30
tssd64		= 0x38
