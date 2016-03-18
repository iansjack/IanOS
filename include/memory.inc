#=============
# Memory Map
#=============
GDT	     		= 0x0000000000002000
IDT				= 0x0000000000002800
OSCode	     	= 0x0000000000180000
OSData	     	= 0x00000000001A8000
OSHeap			= 0x00000000001AA000
TempUserCode 	= 0x000000007D000000
TempUserData 	= 0x000000007E000000
TempUStack   	= 0x000000007F000000
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
