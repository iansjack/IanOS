include 'kstructs.asm'
;=============
; Memory Map
;=============
GDT	     = 0x0000000000000000
IDT	     = 0x0000000000000800
OSCode	     = 0x0000000000001000
OSData	     = 0x0000000000010000
StaticPort   = 0x000000000006F000
DiskBuffer   = 0x0000000000070000
PageMap      = 0x000000000007F000
TaskStruct   = 0x0000000000080000
TempPTL4     = 0x0000000000090000
TempPTL3     = 0x0000000000091000
TempPTL2     = 0x0000000000092000
TempPTL11    = 0x0000000000093000
TempPTL12    = 0x0000000000094000
TempUserCode = 0x0000000000095000
TempUserData = 0x0000000000096000
PageTableL4  = 0x0000000000200000	 ; we will create the tables and fill in
PageTableL3  = 0x0000000000201000	 ; the physical addresses
PageTableL2  = 0x0000000000202000	 ;
PageTableL11 = 0x0000000000203000	 ;
PageTableL12 = 0x0000000000204000	 ;
UserCode     = 0x0000000000300000
UserData     = 0x0000000000310000
KernelStack  = 0x00000000003FC000
UserStack    = 0x00000000003FE000
;      .                                 ;
; Free         0x0000000000400000 - 0xFFFFFFFFFFFFFFFF
PageSize = 0x1000
KbdPort = StaticPort + MP.size

OsCodeSeg 	= 0x8
OsDataSeg 	= 0x10
code64	  	= 0x18
data64		= 0x20
udata64		= 0x28
user64		= 0x30
tssd64		= 0x38


