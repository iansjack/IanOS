#ifndef FAT_H
#define FAT_H

struct DirEntry
{
	char name[11];
	unsigned char attribute;
	unsigned char reserved[10];
	unsigned char date[4];
	short int startingCluster;
	int fileSize;
};

struct BootSector
{
	char		jump[3];
	char		oemLable[8];
	unsigned short	bytesPerSector;
	unsigned char	sectorsPerCluster;
	unsigned short	reservedSectors;
	unsigned char	fats;
	unsigned short	rootEntries;
	unsigned short	smallSectors;
	unsigned char	mediaDescriptor;
	unsigned short	sectorsPerFat;
	unsigned short	sectorsPerTrack;
	unsigned short	heads;
	unsigned int	hiddenSectors;
	unsigned int	largeSectors;
};

struct PartTable
{
	unsigned char	status;
	unsigned char	sectorAddress[3];
	unsigned char	partitionType;
	unsigned char	lastSectorAddress[3];
	unsigned int	LBA;
	unsigned int	sectorsInPartition;
};

struct MBR
{
	char	code[0x1B8];
	char	diskSignature[4];
	char	nulls[2];
	struct	PartTable PT[4];
};

unsigned long FirstFAT;
unsigned long RootDir;
unsigned long DataStart;
unsigned short BytesPerSector;
unsigned char SectorsPerCluster;
unsigned char * DiskBuffer;

#endif
