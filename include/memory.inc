#=============
# Memory Map
#=============
GDT	     		= 0x0000000000000000
IDT				= 0x0000000000000800
OSCode	     	= 0x0000000000001000
OSData	     	= 0x0000000000010000
OSHeap			= 0x0000000000011000
TempUserCode 	= 0x00000000000FD000
TempUserData 	= 0x00000000000FE000
TempUStack   	= 0x00000000000FF000
PageMap      	= 0x0000000000100000
UserCode     	= 0x0000000000400000
UserData     	= 0x0000000000600000
KernelStack  	= 0x00000000009ff000
UserStack    	= 0x0000000000bff000

PageSize	= 0x1000

OsCodeSeg 	= 0x8
OsDataSeg 	= 0x10
code64	  	= 0x18
data64		= 0x20
udata64		= 0x28
user64		= 0x30
tssd64		= 0x38
