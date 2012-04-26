#ifndef FAT_H
#define FAT_H

struct DirEntry {
	unsigned char name[11];
	unsigned char attribute;
	unsigned char reserved[10];
	unsigned short modifiedTime;
	unsigned short modifiedDate;
	short int startingCluster;
	int fileSize;
};

struct BootSector {
	char jump[3];
	char oemLable[8];
	unsigned short bytesPerSector;
	unsigned char sectorsPerCluster;
	unsigned short reservedSectors;
	unsigned char fats;
	unsigned short rootEntries;
	unsigned short smallSectors;
	unsigned char mediaDescriptor;
	unsigned short sectorsPerFat;
	unsigned short sectorsPerTrack;
	unsigned short heads;
	unsigned int hiddenSectors;
	unsigned int largeSectors;
};

struct PartTable {
	unsigned char status;
	unsigned char sectorAddress[3];
	unsigned char partitionType;
	unsigned char lastSectorAddress[3];
	unsigned int LBA;
	unsigned int sectorsInPartition;
};

struct MBR {
	char code[0x1B8];
	char diskSignature[4];
	char nulls[2];
	struct PartTable PT[4];
};

struct vDirNode {
	unsigned char *name;
	unsigned long startSector;
	struct vDirNode *parent;
	struct vDirNode *nextSibling;
	struct vDirNode *firstChild;
};

struct DirSects {
	int cluster;
	int sector;
	int sectorNo;
	int sectorInCluster;
	struct vDirNode *directory;
};

unsigned long FirstFAT;
unsigned long RootDir;
unsigned long DataStart;
unsigned long FATLength;
unsigned short BytesPerSector;
unsigned char SectorsPerCluster;
unsigned short RootDirectoryEntries;

#endif
