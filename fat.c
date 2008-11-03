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
	struct MBR * mbr;
	struct BootSector * bs;
	unsigned int bootSector = 0;
	
	ReadSector(DiskBuffer, 0L);
	mbr = (struct MBR *) DiskBuffer;
	bootSector = (mbr->PT[0]).LBA;
	ReadSector(DiskBuffer, (long)bootSector);
	bs = (struct BootSector *) DiskBuffer;
	RootDir = (bs->sectorsPerFat) * 2 + bs->reservedSectors	+ bootSector;
	DataStart = (bs->rootEntries) / 16 + RootDir;
	SectorsPerCluster = bs->sectorsPerCluster;
}

/*
==============================================================================
Find the root directory entry for the file whose name is pointed to by "name".
Return the address of the directory entry
==============================================================================
*/
long FindFile(char name[11])
{
	short int done = 1;
	struct DirEntry * entries;
	
	ReadSector(DiskBuffer, RootDir);
	entries = (struct DirEntry *) DiskBuffer;
	short int n;
	for (n = 0; n < 16; n++)
	{
		done = 0;
		short int i;
		for (i = 0; i < 11; i++)
			if (entries[n].name[i] != name[i]) 
				done = 1;
		if (done == 0) return (long) &entries[n];
	}
	return 0;
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
	struct FCB fHandle;
	
	OpenFile(name, &fHandle);
	int size = fHandle.length;
	int count = 0;
	ReadFile(&fHandle, DiskBuffer, size);
	//CloseFile(&fHandle);
}
