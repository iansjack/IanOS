#include "cmemory.h"
#include "ckstructs.h"
#include "fat.h"

extern struct Task * currentTask;

/*
==============================
Convert a cluster to a sector.
==============================
*/
unsigned int ClusterToSector(int cluster)
{
	return((cluster - 2) * SectorsPerCluster + DataStart);
}

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
	
	DiskBuffer = AllocKMem(512);
	
	ReadSector(DiskBuffer, 0L);
	mbr = (struct MBR *) DiskBuffer;
	bootSector = (mbr->PT[0]).LBA;
	FirstFAT = bootSector + 1;
	ReadSector(DiskBuffer, (long)bootSector);
	bs = (struct BootSector *) DiskBuffer;
	RootDir = (bs->sectorsPerFat) * 2 + bs->reservedSectors	+ bootSector;
	DataStart = (bs->rootEntries) / 16 + RootDir;
	SectorsPerCluster = bs->sectorsPerCluster;
	BytesPerSector = bs->bytesPerSector;
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
		fHandle->currentCluster = entry->startingCluster;
		fHandle->clusterList = AllocKMem(sizeof(struct clusterListEntry));															  
		fHandle->clusterList->next = 0;
		fHandle->clusterList->cluster = fHandle->currentCluster;
		// Fill in the FAT-chain list
		if (entry->fileSize > BytesPerSector * SectorsPerCluster)
		{
			fHandle->currentClusterEntry = fHandle->clusterList;
			int currentCluster = fHandle->currentClusterEntry->cluster;
			while (currentCluster != 0xFFFF)
			{
				unsigned short * DiskBuffer = AllocKMem(512);
				ReadSector((unsigned char *)DiskBuffer, FirstFAT + (currentCluster / (BytesPerSector / 2)));
				int nextCluster = DiskBuffer[currentCluster % (BytesPerSector / 2)];
				if (nextCluster != 0xFFFF)
				{
					fHandle->currentClusterEntry->next = AllocKMem(sizeof(struct clusterListEntry));
					fHandle->currentClusterEntry = fHandle->currentClusterEntry->next;
					fHandle->currentClusterEntry->cluster = nextCluster;
				}
				currentCluster = nextCluster;
			}
		}
		fHandle->startSector = ClusterToSector(entry->startingCluster);
		fHandle->fileCursor = fHandle->bufCursor = 0;
		fHandle->length = entry->fileSize;
		fHandle->filebuf = (char *)AllocKMem(512);
		ReadSector(fHandle->filebuf, fHandle->startSector);
		fHandle->nextSector = fHandle->startSector + 1;
		fHandle->sectorInCluster = 1;
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
	if (noBytes > fHandle->length) return 0;
	
	long bytesRead = 0;
	
	while (bytesRead < noBytes)
	{
		while (fHandle->bufCursor < 512 && bytesRead < noBytes)
		{
			buffer[bytesRead] = fHandle->filebuf[fHandle->bufCursor];
			bytesRead++;
			fHandle->bufCursor++;
		}
		// If we have read past the end of the buffer we need to load the
		// next sector into the buffer. 
		if (fHandle->bufCursor == 512 && bytesRead < noBytes) 
		{
			if (fHandle->sectorInCluster++ > SectorsPerCluster)
			{
				fHandle->currentClusterEntry = fHandle->currentClusterEntry->next;
				fHandle->currentCluster = fHandle->currentClusterEntry->cluster;
				fHandle->nextSector = ClusterToSector(fHandle->currentCluster);
				fHandle->sectorInCluster = 1;
			}
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
