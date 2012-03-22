#include "memory.h"
#include "kstructs.h"
#include "kernel.h"
#include "fat.h"
#include "filesystem.h"

extern struct Task *currentTask;
struct DirEntry *FindFreeDirEntry(struct DirEntry *);
struct DirEntry *FindFile(unsigned char *name);
int FindFileDirectorySector(unsigned char *filename, struct vDirNode *directory, struct DirEntry *buffer, int *entryOffset);
void CreateVDirectory(void);
unsigned char *DirNameToName(unsigned char dirname[11]);

unsigned char *RootDirBuffer;
struct vDirNode *vDirectory;

//===============================
// Convert a cluster to a sector
//===============================
unsigned int ClusterToSector(int cluster)
{
	return ((cluster - 2) * SectorsPerCluster + DataStart);
}

//===============================
// Convert a sector to a cluster
//===============================
unsigned int SectorToCluster(int sector)
{
	return (sector - DataStart) / SectorsPerCluster + 2;
}

//=====================================
// Read in some parameters from the HD
//=====================================
void InitializeHD(void)
{
	struct MBR *mbr;
	struct BootSector *bs;
	unsigned int bootSector = 0;
	unsigned char *FATbuffer;

	DiskBuffer = (unsigned char *)AllocUMem(512);

	ReadSector(DiskBuffer, 0L);
	mbr = (struct MBR *)DiskBuffer;
	bootSector = (mbr->PT[0]).LBA;
	ReadSector(DiskBuffer, (long)bootSector);
	bs = (struct BootSector *)DiskBuffer;
	RootDir = (bs->sectorsPerFat) * 2 + bs->reservedSectors + bootSector;
	RootDirectoryEntries = bs->rootEntries;
	DataStart = (RootDirectoryEntries) / 16 + RootDir;
	SectorsPerCluster = bs->sectorsPerCluster;
	BytesPerSector = bs->bytesPerSector;
	FirstFAT = bootSector + bs->reservedSectors;
	FATLength = (RootDir - FirstFAT) / 2;
	DeallocMem(DiskBuffer);

	// Read FAT from disk to FAT buffer
	FATbuffer = AllocUMem(FATLength * BytesPerSector);
	int count;
	for (count = 0; count < FATLength; count++) {
		ReadSector(FATbuffer + (count * BytesPerSector),
			   FirstFAT + count);
	}
	FAT = (unsigned short *)FATbuffer;

	// Read root directory from disk to buffer
	RootDirBuffer = AllocUMem(RootDirectoryEntries * 0x20);
	for (count = 0; count < (RootDirectoryEntries * 0x20) / BytesPerSector;
	     count++) {
		ReadSector(RootDirBuffer + (count * BytesPerSector),
			   RootDir + count);
	}
	CreateVDirectory();
}

//============================================================
// Scan the hard disk and create the vDirectory tree
//============================================================
void CreateVDirectory(void)
{
	struct vDirNode *currentDir;

	vDirectory = AllocUMem(sizeof(struct vDirNode));
	vDirectory->name = AllocUMem(2);
	strcpy(vDirectory->name, "/");
	vDirectory->startSector = RootDir;
	vDirectory->parent = 0;
	vDirectory->nextSibling = 0;
	vDirectory->firstChild = 0;
	currentDir = vDirectory;
	GetVDirEntries(currentDir);
}

//=======================================================
// Create the subtree of vDirNodes for the current node
//=======================================================
int GetVDirEntries(struct vDirNode *currentDir)
{
	int i;

	struct DirEntry *dirBuffer = AllocUMem(sizeof(struct DirEntry[16]));
	ReadSector((unsigned char *)dirBuffer, currentDir->startSector);
	for (i = 0; i < 16; i++) {
		if (dirBuffer[i].attribute & 0x10) {
			// It's a directory!!!
			struct vDirNode *temp =
			    AllocUMem(sizeof(struct vDirNode));
			temp->name = DirNameToName(dirBuffer[i].name);
			temp->startSector =
			    ClusterToSector(dirBuffer[i].startingCluster);
			temp->parent = currentDir;
			temp->nextSibling = 0;
			temp->firstChild = 0;
			// If the currentDirectory has no children, make this firstChild
			if (!currentDir->firstChild)
				currentDir->firstChild = temp;
			else {
				// Make it a sibling of a child
				struct vDirNode *temp2 = currentDir->firstChild;
				while (temp2->nextSibling)
					temp2 = temp2->nextSibling;
				temp2->nextSibling = temp;
			}
			// If it's not . or .. recurse
			if (strcmp(temp->name, ".") && strcmp(temp->name, ".."))
				GetVDirEntries(temp);
		}
	}
	DeallocMem(dirBuffer);
}

//==========================================================
// Convert filename as a string to the Directory Entry form
//      Returns 0 on success
//              1 on failure
//==========================================================
int NameToDirName(char *name, char *dirname)
{
	short int i = 0;
	short int j = 0;

	for (i = 0; i < 11; i++) {
		dirname[i] = ' ';
	}
	if (name[0] == '.' && name[1] == '.'
	    && (name[3] == 0 || name[3] == ' ')) {
		dirname[0] = dirname[1] = '.';
		return 0;
	}
	i = 0;
	while (name[i] != '.' && i < 8 && name[i] != 0) {
		dirname[j++] = name[i++];
	}
	if (name[i] == 0)
		return (0);
	if ((i == 8) && (name[i] != '.')) {
		return (1);
	}
	j = 8;
	i++;
	while (name[i] != 0 && j < 11) {
		dirname[j++] = name[i++];
	}
	return (0);
}

//==========================================================
// Convert filename as Directory Entry form to a string
//      Returns a pointer to the string
//==========================================================
unsigned char *DirNameToName(unsigned char dirname[11])
{
	char name[12];
	short int i = 0;
	short int j = 0;

	for (i = 0; i < 11; i++) {
		if (dirname[i] && dirname[i] != ' ') {
			if (i == 8)
				name[j++] = '.';
			name[j++] = dirname[i];
		}
	}
	name[j] = 0;
	unsigned char *retval = AllocUMem(strlen(name) + 1);
	strcpy(retval, name);
	return retval;
}

//============================================================================
// Find virtual directory node of a file. If not found return 0.
//============================================================================
struct vDirNode *FindDirectory(unsigned char *fullpath)
{
	if (fullpath[0] == '/') fullpath++;

	struct vDirNode *temp = vDirectory;
	unsigned char *restOfString;
	while (1) {
		restOfString = strchr(fullpath, '/');
		if (!restOfString)	// Found it
			return temp;
		restOfString[0] = 0;
		restOfString++;
		temp = temp->firstChild;
		while (strcmp(temp->name, fullpath)) {
			temp = temp->nextSibling;
			if (!temp) 
				return 0;
		}
		fullpath = restOfString;
	}
	return 0;
}

//===========================================================================
// Find the directory entry for the file whose name is pointed to by "name".
// Return the address of the directory entry, or 0 on failure. 
//===========================================================================
struct DirEntry *FindFile(unsigned char *name)
{
	struct DirEntry *buffer = AllocUMem(512);
	struct DirEntry *entry = AllocKMem(sizeof(struct DirEntry));
	int entryOffset = 0;
	
	unsigned char *tempName = AllocKMem(strlen(name) + 1);
	strcpy(tempName, name);

	int i = strlen(tempName);
	while (name[i] != '/') i--;
	unsigned char *filename = tempName + i + 1;

	struct vDirNode *directory = FindDirectory(tempName);
	DeallocMem(tempName);
	
	if (!FindFileDirectorySector(filename, directory, buffer, &entryOffset))
	{
		DeallocMem(entry);
		DeallocMem(buffer);
		return 0;
	}
	copyMem((unsigned char *)(buffer + entryOffset), (unsigned char *)entry, sizeof(struct DirEntry));
	DeallocMem(buffer);
	return entry;
}

//======================================================================================================
// Find the directory details of the file name.
// Returns the sector on the disk conatianing its directory entry.
// Sets buffer, containing that sector, to an array of DirEntries, one of which is that for the file.
// Sets entry to a pointer to the actual directory entry within that sector
//======================================================================================================
int FindFileDirectorySector(unsigned char *filename, struct vDirNode *directory, struct DirEntry *buffer, int *entryOffset)
{
	int sector = 0;
	unsigned char *dirName = 0;

	struct DirSects dirSector;
	sector = FindFirstDirectorySector (directory, &dirSector);
	if (sector)
	{
		ReadSector((unsigned char *)buffer, sector);
		struct DirEntry *temp = buffer;

		while (sector)
		{
			int entriesInSect = BytesPerSector / sizeof (struct DirEntry);
			int entryno = 0;
			while (entryno < entriesInSect)
			{
				dirName = DirNameToName(temp->name);
				if (!strcmp(dirName, filename))
				    break;
				entryno++;
				temp++;
			}
			if (entryno != entriesInSect)
			{
				*entryOffset = entryno;
				break;
			}
			sector = FindNextDirectorySector (&dirSector);
			entryno = 0;
		}
	}
	DeallocMem(dirName);
	return (sector);
}

//=================================
// Finds next free cluster on disk
//=================================
int FindFreeCluster(void)
{
	int count = 0;

	while (FAT[count] != 0) {
		count++;
	}
	return (count);
}

//==========================================
// Find a free directory entry in directory
// Return a pointer to the directory entry
//        or 0 on failure
//==========================================
struct DirEntry *FindFreeDirEntry(struct DirEntry *directory)
{
	int count = 0;
	struct DirEntry *entries;

	entries = directory;
	while (entries[count].name[0] != 0xE5) {
		if (entries[count].name[0] == 0)
			return (&entries[count]);
		count++;
		if (count == RootDirectoryEntries)
			return (0);
	}
	return (&entries[count]);
}

//==================================
// Save the FAT buffer back to disk
//==================================
void SaveFAT(void)
{
	char *FATbuffer = (char *)FAT;
	int count;

	for (count = 0; count < FATLength; count++)
		WriteSector(FATbuffer + (count * BytesPerSector),
			    FirstFAT + count);
}

//========================================
// Save the Directory buffer back to disk
//========================================
void SaveDir(struct vDirNode *directory)
{
	int count = 0;

	for (count = 0; count < (RootDirectoryEntries * 0x20) / BytesPerSector;
	     count++)
		WriteSector((unsigned char *)directory->startSector + (count * BytesPerSector), RootDir + count);
}

//======================================
// Create a new file.
// Returns 1 on success
//       0 if file cannot be created
//======================================
struct FCB *CreateFile(unsigned char *name, unsigned short pid)
{
	// If the file already exist, exit
	if (FindFile(name)) return (0);

	struct FCB *fHandle = (struct FCB *)AllocKMem(sizeof(struct FCB));
	struct vDirNode *dir = FindDirectory(name);
	struct DirEntry *directory = AllocUMem(512);

	unsigned char *tempName = AllocKMem(strlen(name) + 1);
	strcpy(tempName, name);

	int i = strlen(tempName);
	while (name[i] != '/') i--;
	unsigned char * fileName = tempName + i + 1;
	           
	if (dir) {
		struct DirSects dirSect;
		int sector = FindFirstDirectorySector(directory, &dirSect);
		ReadSector((unsigned char *)directory, sector);
		struct DirEntry *entry = directory;
		int count;
		
		// Find a free entry
		while (!(entry->name[0] == 0xE5 || entry->name[0] == 0)) 
		{
			entry++;
			if (entry == directory + 512)
			{
				sector = FindNextDirectorySector(&dirSect);  // Need some code here to deal with the fact that there are no free entries
				ReadSector((unsigned char *)directory, sector);
			}
		}
		
		// Zero directory entry
		for (count = 0; count < sizeof(struct DirEntry); count++)
			((unsigned char *)entry)[count] = (unsigned char)0;

		// Fill in a few details
		if (NameToDirName(fileName, entry->name)) {
			DeallocMem(fHandle);
			DeallocMem(directory);
			DeallocMem(tempName);
			return (0);
		}

		entry->startingCluster = FindFreeCluster();
		entry->attribute = 0x20;

		// Mark the cluster as in use
		FAT[entry->startingCluster] = 0xFFFF;

		SaveFAT();
		// SaveDir(directory);
		WriteSector((unsigned char *)directory, dir->startSector);
		fHandle->dirEntry = entry;
		fHandle->dir = FindDirectory(name);
		fHandle->currentCluster = entry->startingCluster;
		fHandle->startSector = ClusterToSector(entry->startingCluster);
		fHandle->startCluster = entry->startingCluster;
		fHandle->fileCursor = fHandle->bufCursor = fHandle->bufIsDirty = 0;
		fHandle->length = entry->fileSize;
		fHandle->filebuf = (char *)AllocUMem(512);
		fHandle->sectorInCluster = 1;
		DeallocMem(directory);
		DeallocMem(tempName);
		return (fHandle);
	}
	else {
		DeallocMem(fHandle);
		DeallocMem(directory);
		DeallocMem(tempName);
		return (0);
	}
}

//===================================================================
// Open a file.
// Returns fHandle on success.
//       0 if file does not exist
//===================================================================
struct FCB *OpenFile(unsigned char *name, unsigned short pid)
{
	struct FCB *fHandle = AllocKMem(sizeof(struct FCB));

	// Root directory isn't a normal file
	if (!strcmp(name, "/")) {
		fHandle->deviceType = DIR;
		fHandle->dirEntry = 0;
		fHandle->dir = 0;
		fHandle->currentCluster = 0;
		fHandle->startSector = RootDir;
		fHandle->startCluster = 0;
		fHandle->fileCursor = fHandle->bufCursor = fHandle->bufIsDirty =
		    0;
		fHandle->length = 512;
		fHandle->filebuf = (char *)AllocUMem(512);
		ReadSector(fHandle->filebuf, fHandle->startSector);
		fHandle->nextSector = fHandle->startSector + 1;
		fHandle->sectorInCluster = 1;
		return (fHandle);
	} else {
		struct DirEntry *entry = (struct DirEntry *)FindFile(name);

		if (entry != 0) {
			fHandle->dirEntry = entry;
			fHandle->dir = FindDirectory(name);
			fHandle->currentCluster = entry->startingCluster;
			fHandle->startSector =
			    ClusterToSector(entry->startingCluster);
			fHandle->startCluster = entry->startingCluster;
			fHandle->fileCursor = fHandle->bufCursor =
			    fHandle->bufIsDirty = 0;
			if (entry->attribute & 0x10) {
				fHandle->deviceType = DIR;
				fHandle->length = 512;
			} else {
				fHandle->length = entry->fileSize;
				fHandle->deviceType = FILE;
			}
			fHandle->filebuf = (char *)AllocUMem(512);
			ReadSector(fHandle->filebuf, fHandle->startSector);
			fHandle->nextSector = fHandle->startSector + 1;
			fHandle->sectorInCluster = 1;
			return (fHandle);
		} else {
			DeallocMem(fHandle);
			return (0);
		}
	}
}

//============================================
// Close a file and release all its resources
//============================================
void CloseFile(struct FCB *fHandle)
{
	int offset;
	unsigned char *filename = DirNameToName(fHandle->dirEntry->name);
 
	if (fHandle->bufIsDirty) {
		WriteSector(fHandle->filebuf,
			    ClusterToSector(fHandle->currentCluster) +
			    fHandle->sectorInCluster - 1);
	}
	if (fHandle->deviceType == FILE || fHandle->deviceType == DIR) {
		if (!fHandle->dirEntry)	// No dirEntry so it must be the root directory
		{
		}
		else
		{
			struct DirEntry *buffer = AllocUMem(512);
			int sector = FindFileDirectorySector(filename, fHandle->dir, buffer, &offset);
			DeallocMem(filename);
			struct DirEntry *temp = buffer + offset;
			// SaveDir(); We need to write values to the directory
			if (fHandle->deviceType == FILE)
			{
				temp->fileSize = fHandle->length;
				WriteSector((unsigned char *)buffer, sector);
			}
			DeallocMem(fHandle->dirEntry);
			DeallocMem(buffer);
		}
		DeallocMem((struct MemStruct *)fHandle->filebuf);
	}
	DeallocMem(fHandle);
}

//=======================================================================
// Find the first sector of a directory
//=======================================================================
int FindFirstDirectorySector(struct vDirNode *directory, struct DirSects *dirSect)
{
	dirSect->sectorNo = 1;
	dirSect->sectorInCluster = 1;
	dirSect->cluster = SectorToCluster(directory->startSector);
	dirSect->directory = directory;
	dirSect->sector = directory->startSector;
	return dirSect->sector;
}

//========================================================================
// Find the next sector of a directory (first do FindFirstSect()
//========================================================================
int FindNextDirectorySector(struct DirSects *dirSect)
{
	dirSect->sectorNo++;
	dirSect->sectorInCluster++;
	if (dirSect->sectorInCluster < SectorsPerCluster)
		dirSect->sector++;
	else
	{
		if (dirSect->directory->parent == 0) // The root directory
		{
			dirSect->sector++;
			if (dirSect->sector > RootDirectoryEntries * sizeof(struct DirEntry) / BytesPerSector)
				dirSect->sector = 0;
		}
		else	// it's a normal directory so get next cluster
		{
			dirSect->cluster = FAT[dirSect->cluster];
			dirSect->sector =	ClusterToSector(dirSect->cluster);
			dirSect->sectorInCluster = 1;
		}	
	}
	return dirSect->sector;
}

//===============================================================
// Read noBytes from the file represented by fHandle into buffer
// Returns no of bytes read
//===============================================================
long ReadFile(struct FCB *fHandle, char *buffer, long noBytes)
{
	long bytesRead = 0;

	while (bytesRead < noBytes && fHandle->bufCursor < fHandle->length) {
		while (fHandle->bufCursor < 512 && bytesRead < noBytes && fHandle->bufCursor < fHandle->length) {
			buffer[bytesRead] =
			    fHandle->filebuf[fHandle->bufCursor];
			bytesRead++;
			fHandle->bufCursor++;
		}
		if (fHandle->bufCursor == fHandle->length)
			break;

		// If we have read past the end of the buffer we need to load the
		// next sector into the buffer.
		if ((fHandle->bufCursor == 512) && (bytesRead < noBytes)) {
			if (fHandle->bufIsDirty) {
				WriteSector(fHandle->filebuf,
					    ClusterToSector
					    (fHandle->currentCluster)
					    + fHandle->sectorInCluster - 1);
			}
			if (fHandle->sectorInCluster++ == SectorsPerCluster) {
				Debug();
				fHandle->currentCluster =
				    FAT[fHandle->currentCluster];
				fHandle->nextSector =
				    ClusterToSector(fHandle->currentCluster);
				fHandle->sectorInCluster = 1;
			}
			ReadSector(fHandle->filebuf, fHandle->nextSector);
			fHandle->bufCursor = 0;
			fHandle->nextSector++;
		}
	}
	return (bytesRead);
}

//================================================================
// Write noBytes from buffer into the file represented by fHandle
// Returns no of bytes written
//================================================================
long WriteFile(struct FCB *fHandle, char *buffer, long noBytes)
{
	long bytesWritten = 0;

	while (bytesWritten < noBytes) {
		while (fHandle->bufCursor < 512 && bytesWritten < noBytes) {
			fHandle->filebuf[fHandle->bufCursor] =
			    buffer[bytesWritten];
			fHandle->bufIsDirty = 1;
			bytesWritten++;
			fHandle->length++;
			fHandle->bufCursor++;
		}

		// If we have written past the end of the buffer we need to flush the sector to the disk
		if ((fHandle->bufCursor == 512) && (bytesWritten < noBytes)) {
			WriteSector(fHandle->filebuf,
				    ClusterToSector(fHandle->currentCluster)
				    + fHandle->sectorInCluster - 1);
			if (fHandle->sectorInCluster++ > SectorsPerCluster) {
				// Allocate another cluster
				int freeCluster = FindFreeCluster();
				FAT[fHandle->currentCluster] = freeCluster;
				FAT[freeCluster] = 0xFFFF;
				fHandle->currentCluster = freeCluster;
				fHandle->nextSector =
				    ClusterToSector(fHandle->currentCluster);
				fHandle->sectorInCluster = 1;
			}
			fHandle->bufCursor = 0;
			fHandle->nextSector++;
		}
	}
	return (bytesWritten);
}

//===================================
// Delete a file.
// Returns 0 on success
//       1 if file does not exist
//===================================
int DeleteFile(char name[11], unsigned short pid)
{
	struct DirEntry *entry = (struct DirEntry *)FindFile(name);

	if (entry != 0) {
		entry->name[0] = 0xE5;
		unsigned short int cluster = entry->startingCluster;
		unsigned short int nextCluster;
		while (cluster != 0xFFFF) {
			nextCluster = FAT[cluster];
			FAT[cluster] = 0;
			cluster = nextCluster;
		}
		SaveFAT();
		struct FCB *fHandle = 0;
		struct DirEntry * directory = AllocUMem(512);
		struct DirSects dirSector;
		int sector = FindFirstDirectorySector(fHandle->dir, &dirSector);
		if (sector)
		{
			ReadSector((unsigned char *)directory, sector);
			struct DirEntry *temp = directory;

			while (sector)
			{
				int entriesInSect = BytesPerSector / sizeof (struct DirEntry);
				int entryno = 0;
				while (temp->startingCluster != fHandle->startCluster && entryno < entriesInSect)
				{
					entryno++;
					temp++;
				}
				if (entryno != entriesInSect)
					break;
				sector = FindNextDirectorySector (&dirSector);
				entryno = 0;
			}
			// SaveDir(); We need to write values to the directory
			if (fHandle->deviceType == FILE)
			{
				temp->name[0] = 0xE5;
				WriteSector((unsigned char *)directory, sector);
			}
			if (fHandle->dirEntry)
				DeallocMem(fHandle->dirEntry);
			DeallocMem((struct MemStruct *)fHandle->filebuf);
		}
		DeallocMem(directory);
		return (0);
	} else {
		return (1);
	}
}

//=============================
// The actual filesystem task
//=============================
void fsTaskCode(void)
{
	KWriteString("Starting Filesystem Task", 3, 0);

	struct Message *FSMsg;
	struct MessagePort *tempPort;

	FSMsg = (struct Message *)ALLOCMSG;

	int result;
	struct FCB *fcb;

	((struct MessagePort *)FSPort)->waitingProc = (struct Task *)-1L;
	((struct MessagePort *)FSPort)->msgQueue = 0;

	InitializeHD();

	while (1) {
		ReceiveMessage((struct MessagePort *)FSPort, FSMsg);
		switch (FSMsg->byte) {
		case CREATEFILE:
			;
			struct FCB *fcb =
			    CreateFile((char *)FSMsg->quad, FSMsg->pid);
			tempPort = (struct MessagePort *)FSMsg->tempPort;
			if (!fcb)
				FSMsg->quad = 0;
			else {
				fcb->pid = FSMsg->pid;
				FSMsg->quad = (long)fcb;
			}
			SendMessage(tempPort, FSMsg);
			break;

		case OPENFILE:
			fcb =
			    OpenFile((unsigned char *)FSMsg->quad, FSMsg->pid);
			tempPort = (struct MessagePort *)FSMsg->tempPort;
			if (!fcb)
				FSMsg->quad = 0;
			else {
				fcb->pid = FSMsg->pid;
				FSMsg->quad = (long)fcb;
			}
			SendMessage(tempPort, FSMsg);
			break;

		case CLOSEFILE:
			CloseFile((struct FCB *)FSMsg->quad);
			DeallocMem((void *)FSMsg->quad);
			tempPort = (struct MessagePort *)FSMsg->tempPort;
			SendMessage(tempPort, FSMsg);
			break;

		case READFILE:
			result =
			    ReadFile((struct FCB *)FSMsg->quad,
				     (char *)FSMsg->quad2, FSMsg->quad3);
			tempPort = (struct MessagePort *)FSMsg->tempPort;
			FSMsg->quad = result;
			SendMessage(tempPort, FSMsg);
			break;

		case WRITEFILE:
			result =
			    WriteFile((struct FCB *)FSMsg->quad,
				      (char *)FSMsg->quad2, FSMsg->quad3);
			tempPort = (struct MessagePort *)FSMsg->tempPort;
			FSMsg->quad = result;
			SendMessage(tempPort, FSMsg);
			break;

		case DELETEFILE:
			result = DeleteFile((char *)FSMsg->quad, FSMsg->pid);
			tempPort = (struct MessagePort *)FSMsg->tempPort;
			SendMessage(tempPort, FSMsg);
			break;

		case TESTFILE:
			if (!strcmp(FSMsg->quad, "/"))
				result = 1;
			else {
				result =
				    (long)FindFile((char *)FSMsg->quad);
				if (result) {
					DeallocMem((void *)((long)result));
					result = 1;
				}
			}

			tempPort = (struct MessagePort *)FSMsg->tempPort;
			FSMsg->quad = result;
			SendMessage(tempPort, FSMsg);
			break;

		case GETPID:
			FSMsg->quad = currentTask->pid;
			SendMessage(tempPort, FSMsg);
			break;

		case GETFILEINFO:
			;
			struct FileInfo info;
			fcb = (struct FCB *)FSMsg->quad;
			info.Length = fcb->length;
			int count;
			for (count = 0; count < sizeof(struct FileInfo);
			     count++)
				((char *)FSMsg->quad2)[count] =
				    ((char *)(&info))[count];
			tempPort = (struct MessagePort *)FSMsg->tempPort;
			SendMessage(tempPort, FSMsg);
			break;

		default:
			break;
		}
	}
}
