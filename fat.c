#include "cmemory.h"
#include "ckstructs.h"
#include "fat.h"

extern struct Task * currentTask;

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
	char * buffer = (char *) DiskBuffer;
	struct MBR * mbr;
	struct BootSector * bootSect;
	unsigned int bootSector = 0;
	
	ReadSector(buffer, 0L);
	mbr = (struct MBR *) buffer;
	bootSector = (mbr->PT[0]).LBA;
	ReadSector(buffer, (long)bootSector);
	bootSect = (struct BootSector *) buffer;
	RootDir = (bootSect->sectorsPerFat) * 2
				+ bootSect->reservedSectors
				+ bootSector;
	DataStart = (bootSect->rootEntries) / 16 + RootDir;
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
	while ((done == 1) && (currentEntry < (struct DirEntry *) DiskBuff + 16))
	{
		currentEntry++;
		done = 0;
		int i;
		for (i = 0; i < 11; i++)
			if (currentEntry->name[i] != name[i]) done = 1;
	}
	if (currentEntry == (struct DirEntry *) DiskBuff + 16) return 0;
	return (long) currentEntry;
}

/*
===================================
Open a file. Fills in FCB info
Returns 0 on success
 		1 if file does not exist
===================================
*/
int OpenFile(char name[11], struct FCB * fHandle)
{
	char * buffer = (char *)DiskBuffer;
	
	ReadSector(buffer, RootDir);
	struct DirEntry * entry = (struct DirEntry *)FindFile(name);
	if (entry != 0) 
	{	
		fHandle->startSector = (entry->startingCluster - 2) * SectorsPerCluster + DataStart;
		fHandle->fileCursor = 0;
		fHandle->bufCursor = 0;
		fHandle->length = entry->fileSize;
		fHandle->filebuf = (char *)AllocMem(512, (struct MemStruct *)(currentTask->firstfreemem));
		ReadSector(fHandle->filebuf, fHandle->startSector);
		fHandle->nextSector = fHandle->startSector + 1;
		return 0;
	}
	else
	{
		return 1;
	}
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
