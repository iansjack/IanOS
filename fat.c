#include "cmemory.h"
#include "ckstructs.h"

extern struct Task * currentTask;

struct DirEntry
{
	char name[11];
	unsigned char attribute;
	unsigned char reserved[10];
	unsigned char date[4];
	short int startingCluster;
	int fileSize;
};

long RootDir;
long DataStart;
char SectorsPerCluster;

/*
======================================================
Read one sector from the hard disk to the disk buffer
======================================================
*/
void ReadSector(char * buffer, long sector)
{
	asm("int $21");
}

/*
=====================================
 Read in some parameters from the HD
=====================================
*/
void InitializeHD()
{
	short sectorsPerFAT = 0;
	short reservedSectors = 0;
	char * buffer = (char *) DiskBuffer;
	char * temp;
	int * buff = 0;
	short * buff2 = 0;
	int bootSector = 0;
	
	ReadSector(buffer, 0L);
	buff = (int *)(buffer + 0x1BE + 8);
	bootSector = (int) buff[0];
	ReadSector(buffer, (long)bootSector);
	SectorsPerCluster = buffer[0xD];
	buff2 = (short *)(buffer + 0x16);
	sectorsPerFAT = buff2[0];
	buff2 = (short *)(buffer + 0x0E);
	reservedSectors = buff2[0];
	RootDir = sectorsPerFAT * 2 + reservedSectors + bootSector;
	buff2 = (short *)(buffer + 0x11);
	DataStart = buff2[0] / 16 + RootDir;
}

/*
============================================================================
Find the root directory entry for the file whose name is pointed to by Name
Return the address of the directory entry
============================================================================
*/
long FindFile(char name[11])
{
	short int done = 1;
	long * DiskBuff = (long *)DiskBuffer;
	struct DirEntry * currentEntry;

	currentEntry = (struct DirEntry *) DiskBuff - 1;
	while (done == 1)
	{
		currentEntry++;
		done = 0;
		int i;
		for (i = 0; i < 11; i++)
			if (currentEntry->name[i] != name[i]) done = 1;
	}
	return (long) currentEntry;
}

/*
===================================
Open a file. Fills in FCB info
===================================
*/
void OpenFile(char name[11], struct FCB * fHandle)
{
	char * buffer = (char *)DiskBuffer;
	
	ReadSector(buffer, RootDir);
	struct DirEntry * entry = (struct DirEntry *)FindFile(name);
	fHandle->startSector = (entry->startingCluster - 2) * SectorsPerCluster + DataStart;
	fHandle->fileCursor = 0;
	fHandle->bufCursor = 0;
	fHandle->length = entry->fileSize;
	fHandle->filebuf = (char *)AllocMem(512, (struct MemStruct *)(currentTask->firstfreemem));
	ReadSector(fHandle->filebuf, fHandle->startSector);
	fHandle->nextSector = fHandle->startSector + 1;
}

/*
============================================
Close a file and release all it's resources
============================================
*/
CloseFile(struct FCB * fHandle)
{
	fHandle->bufCursor = 0;
	fHandle->fileCursor = 0;
	fHandle->length = 0;
	fHandle->nextSector = 0;
	fHandle->startSector = 0;
	DeallocMem((struct MemStruct *)fHandle->filebuf);
}

/*
==============================================================
 Read nBytes from the file represented by fHandle into buffer
 Returns no of bytes read
==============================================================
*/
long ReadFile(struct FCB * fHandle, char * buffer, long noBytes)
{
	long bytesRead = 0;
	
	if (fHandle->bufCursor == 512)
	{
		ReadSector(fHandle->filebuf, fHandle->nextSector);
		fHandle->nextSector++;
	}
	while (bytesRead < noBytes)
	{
		while (fHandle->bufCursor < 512 && bytesRead < noBytes)
		{
			buffer[bytesRead] = fHandle->filebuf[fHandle->bufCursor];
			bytesRead++;
			fHandle->bufCursor++;
		}
		if (fHandle->bufCursor == 512)
		{
			ReadSector(fHandle->filebuf, fHandle->nextSector);
			fHandle->bufCursor = 0;
			fHandle->nextSector++;
		}
	}
	return bytesRead;
}

/*
=====================================
 Load a file into DiskBuffer
=====================================
*/
void LoadFile(char name[11])
{
	char * buffer = (char *)DiskBuffer;
	struct FCB fHandle;
	
	OpenFile(name, &fHandle);
	int size = fHandle.length;
	int count = 0;
	ReadFile(&fHandle, buffer, size);
	//CloseFile(&fHandle);
}
