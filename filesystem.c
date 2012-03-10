#include "memory.h"
#include "kstructs.h"
#include "kernel.h"
#include "fat.h"
#include "filesystem.h"

extern struct Task *currentTask;
struct DirEntry * FindFreeDirEntry(struct DirEntry *);
struct DirEntry * FindFile(unsigned char *name, unsigned short pid);
void CreateVDirectory(void);
unsigned char *DirNameToName(unsigned char dirname[11]);

unsigned char *RootDirBuffer;
struct vDirNode * vDirectory;

//struct DirectoryBuffer DirectoryBuffers[10];

//===============================
// Convert a cluster to a sector
//===============================
unsigned int ClusterToSector(int cluster)
{
   return ((cluster - 2) * SectorsPerCluster + DataStart);
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

   DiskBuffer = (unsigned char *) AllocUMem(512);

   ReadSector(DiskBuffer, 0L);
   mbr = (struct MBR *) DiskBuffer;
   bootSector = (mbr->PT[0]).LBA;
   ReadSector(DiskBuffer, (long) bootSector);
   bs = (struct BootSector *) DiskBuffer;
   RootDir = (bs->sectorsPerFat) * 2 + bs->reservedSectors + bootSector;
   RootDirectoryEntries = bs->rootEntries;
   DataStart = (RootDirectoryEntries) / 16 + RootDir;
   SectorsPerCluster = bs->sectorsPerCluster;
   BytesPerSector = bs->bytesPerSector;
   FirstFAT = bootSector + SectorsPerCluster;
   FATLength = (RootDir - FirstFAT) / 2;

   // Read FAT from disk to FAT buffer
   FATbuffer = AllocUMem(FATLength * BytesPerSector);
   int count;
   for (count = 0; count < FATLength; count++)
   {
      ReadSector(FATbuffer + (count * BytesPerSector), FirstFAT + count);
   }
   FAT = (unsigned short *) FATbuffer;

   // Read root directory from disk to buffer
   RootDirBuffer = AllocUMem(RootDirectoryEntries * 0x20);
   for (count = 0; count < (RootDirectoryEntries * 0x20) / BytesPerSector; count++)
   {
      ReadSector(RootDirBuffer + (count * BytesPerSector), RootDir + count);
   }
	CreateVDirectory();
}

//============================================================
// Scan the hard disk and create the vDirectory tree
//============================================================
void CreateVDirectory(void)
{
	struct vDirNode *currentDir;
	
	vDirectory = AllocUMem(sizeof (struct vDirNode));
	vDirectory->name = AllocUMem(2);
	strcpy(vDirectory->name, "/");
	vDirectory->startSector = RootDir;
	vDirectory->parent = 0;
	vDirectory->nextSibling = 0;
	vDirectory->firstChild = 0;
	currentDir = vDirectory;
	GetVDirEntries(currentDir);
}

int GetVDirEntries(struct vDirNode *currentDir)
{
	int i;
	
	struct DirEntry *dirBuffer = AllocUMem(sizeof (struct DirEntry[16]));
	ReadSector((unsigned char *)dirBuffer, currentDir->startSector);
	for (i = 0; i<16; i++)
	{
		if (dirBuffer[i].attribute & 0x10)
		{
			// It's a directory!!!
			struct vDirNode *temp = AllocUMem(sizeof (struct vDirNode));
			temp->name = DirNameToName(dirBuffer[i].name);
			temp->startSector = ClusterToSector(dirBuffer[i].startingCluster);
			temp->parent = currentDir;
			temp->nextSibling = 0;
			temp->firstChild = 0;
			// If the currentDirectory has no children, make this firstChild
			if (!currentDir->firstChild)
				currentDir->firstChild = temp;
			else
			{
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
}

//==========================================================
// Convert filename as a string to the Directory Entry form
//	Returns 0 on success
//              1 on failure
//==========================================================
int NameToDirName(char *name, char *dirname)
{
   short int i = 0;
   short int j = 0;

   for (i = 0; i < 11; i++)
   {
      dirname[i] = ' ';
   }
   if (name[0] == '.' && name[1] == '.' && (name[3] == 0 || name[3] == ' '))
   {
      dirname[0] = dirname[1] = '.';
      return 0;
   }
   i = 0;
   while (name[i] != '.' && i < 8 && name[i] != 0)
   {
      dirname[j++] = name[i++];
   }
   if (name[i] == 0)
      return (0);
   if ((i == 8) && (name[i] != '.'))
   {
      return (1);
   }
   j = 8;
   i++;
   while (name[i] != 0 && j < 11)
   {
      dirname[j++] = name[i++];
   }
   return (0);
}

//==========================================================
// Convert filename as Directory Entry form to a string
//	Returns a pointer to the string
//==========================================================
unsigned char *DirNameToName(unsigned char dirname[11])
{
	char name[12];
   	short int i = 0;
   	short int j = 0;

	for (i = 0; i < 11; i++)
	{
		if (dirname[i] && dirname[i] != ' ')
		{
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
// Find the array of DirEntrys for directory. If a relative path is specified,
// start looking in the cwd of task with pid pid. If not found return 0.
// If necessary, allocate a buffer for the directory.
//============================================================================
struct DirEntry *FindDirectory(unsigned char *directory, unsigned short pid)
{
	struct DirEntry *retVal = AllocUMem(512);
	unsigned char *fullpath = AllocUMem(256);
	
	if (directory[0] == '/') // Absolute path
		strcpy(fullpath, directory);
	else // Relative path
	{
		strcpy(fullpath, PidToTask(pid)->currentDirName);
		if (strcmp(fullpath, "/"))
			strcat(fullpath, "/");
		strcat(fullpath, directory);
	}
	// Now we have the full path of the file, so we search VDirectory for the directory
	if (fullpath[0] == '/') fullpath++;
	struct vDirNode *temp = vDirectory;
	unsigned char *restOfString;
	while (1)
	{
		restOfString = strchr(fullpath, '/');
		if (!restOfString) // Found it
		{
			DeallocMem(fullpath);
			ReadSector((unsigned char *)retVal, temp->startSector);
			return retVal;
		}
		restOfString[0] = 0;
		restOfString++;
		temp = temp->firstChild;
		while (strcmp(temp->name, fullpath))
			temp = temp->nextSibling;
	}
	DeallocMem(retVal);
	DeallocMem(fullpath);
	return 0;
}

//===========================================================================
// Find the directory entry for the file whose name is pointed to by "name".
// Return the address of the directory entry, or 0 on failure. If a relative
// path is specified start looking in the cwd of task with pid pid
//===========================================================================
struct DirEntry *FindFile(unsigned char *name, unsigned short pid)
{
   	struct DirEntry *entries;
	struct DirEntry *retval = 0;

   	entries = FindDirectory(name, pid);
	if (entries)
	{
		unsigned char *tempname;
		while (tempname = strchr(name, '/')) name = tempname + 1;
   		short int n;
   		for (n = 0; n < RootDirectoryEntries; n++)
   		{
			if (entries[n].name[0] == 0) break;
	   		if (!strcmp(name, DirNameToName(entries[n].name)))
	   		{
				retval = AllocUMem(sizeof (struct DirEntry));
		   		copyMem((unsigned char *)&entries[n], (unsigned char *)retval, sizeof (struct DirEntry));
				break;
	   		}
		}
		DeallocMem(entries);
	}
   	return (retval);
}

//=================================
// Finds next free cluster on disk
//=================================
int FindFreeCluster(void)
{
   int count = 0;

   while (FAT[count] != 0)
   {
      count++;
   }
   return (count);
}

//==========================================
// Find a free directory entry in directory
// Return a pointer to the directory entry
//        or 0 on failure
//==========================================
struct DirEntry * FindFreeDirEntry(struct DirEntry *directory)
{
   int count = 0;
   struct DirEntry *entries;

   entries = directory;
   while (entries[count].name[0] != 0xE5)
   {
      if (entries[count].name[0] == 0)
      {
         return (&entries[count]);
      }
      count++;
      if (count == RootDirectoryEntries)
      {
         return (0);
      }
   }
   return (&entries[count]);
}

//==================================
// Save the FAT buffer back to disk
//==================================
void SaveFAT(void)
{
   char *FATbuffer = (char *) FAT;
   int count;

   for (count = 0; count < FATLength; count++)
   {
      WriteSector(FATbuffer + (count * BytesPerSector), FirstFAT + count);
   }
}

//========================================
// Save the Directory buffer back to disk
//========================================
void SaveDir(void)
{
   int count;
   // char *RootDirBuffer = (char *) DirectoryBuffers[0].Buffer;

   for (count = 0; count < (RootDirectoryEntries * 0x20) / BytesPerSector; count++)
   {
      WriteSector(RootDirBuffer + (count * BytesPerSector), RootDir + count);
   }
}

//======================================
// Create a new file.
// Returns 1 on success
//       0 if file cannot be created
//======================================
struct FCB * CreateFile(char *name, unsigned short pid)
{
   	struct FCB *fHandle = (struct FCB *) AllocKMem(sizeof(struct FCB));

	if (FindFile(name, pid))
   {
      return (0);
   }
   struct DirEntry *entry;
	entry = (struct DirEntry *)RootDirBuffer;
//   if ((entry = FindFreeDirEntry(pid)) == 0)
//   {
//      return (0);
//   }
   int count;

   // Zero directory entry
   for (count = 0; count < sizeof(struct DirEntry); count++)
   {
      ((unsigned char *) entry)[count] = (unsigned char) 0;
   }

   // Fill in a few details
   if (NameToDirName(name, entry->name))
   {
      return (0);
   }
   entry->startingCluster = FindFreeCluster();
   entry->attribute = 0x20;

   // Mark the cluster as in use
   FAT[entry->startingCluster] = 0xFFFF;

   SaveFAT();
   SaveDir();
   fHandle->directory = entry;
   fHandle->currentCluster = entry->startingCluster;
   fHandle->startSector = ClusterToSector(entry->startingCluster);
   fHandle->startCluster = entry->startingCluster;
   fHandle->fileCursor = fHandle->bufCursor = fHandle->bufIsDirty = 0;
   fHandle->length = entry->fileSize;
   fHandle->filebuf = (char *) AllocUMem(512);
   fHandle->sectorInCluster = 1;
   return (fHandle);
}

//===================================================================
// Open a file.
// Returns fHandle on success.
//       0 if file does not exist
//===================================================================
struct FCB * OpenFile(char name[11], unsigned short pid)
{
	struct FCB * fHandle = AllocKMem(sizeof(struct FCB));
	
	// This is a kludge, but bear with it for the time being!
	if (!strcmp(name, "/"))
	{
		fHandle->deviceType = DIR;
		fHandle->directory = 0;
		fHandle->currentCluster = 0;
      	fHandle->startSector = RootDir;
      	fHandle->startCluster = 0;
      	fHandle->fileCursor = fHandle->bufCursor = fHandle->bufIsDirty = 0;
		fHandle->length = 512;
      	fHandle->filebuf = (char *) AllocUMem(512);
      	ReadSector(fHandle->filebuf, fHandle->startSector);
      	fHandle->nextSector = fHandle->startSector + 1;
      	fHandle->sectorInCluster = 1;
      	return (fHandle);		
	}
	else
	{ 
		struct DirEntry *entry = (struct DirEntry *) FindFile(name, pid);

   		if (entry != 0)
   		{
      		fHandle->directory = entry;
      		fHandle->currentCluster = entry->startingCluster;
      		fHandle->startSector = ClusterToSector(entry->startingCluster);
      		fHandle->startCluster = entry->startingCluster;
      		fHandle->fileCursor = fHandle->bufCursor = fHandle->bufIsDirty = 0;
	   		if (entry->attribute & 0x10)
	   		{
		   		fHandle->deviceType = DIR;
		   		fHandle->length = 512;
	   		}
	   		else
	   		{
      			fHandle->length = entry->fileSize;
		   		fHandle->deviceType = FILE;
	   		}
      		fHandle->filebuf = (char *) AllocUMem(512);
      		ReadSector(fHandle->filebuf, fHandle->startSector);
      		fHandle->nextSector = fHandle->startSector + 1;
      		fHandle->sectorInCluster = 1;
      		return (fHandle);
   		}
   		else
   		{
			DeallocMem(fHandle);
      		return (0);
   		}
	}
}

//============================================
// Close a file and release all its resources
//============================================
void CloseFile(struct FCB * fHandle)
{
   if (fHandle->bufIsDirty)
   {
      WriteSector(
            fHandle->filebuf,
            ClusterToSector(fHandle->currentCluster) + fHandle->sectorInCluster
                  - 1);
   }
	if (fHandle->deviceType == FILE)
	{
   		fHandle->directory->fileSize = fHandle->length;
   		SaveDir();
		DeallocMem(fHandle->directory);
   		DeallocMem((struct MemStruct *) fHandle->filebuf);
	}
   DeallocMem(fHandle);
}

//===============================================================
// Read noBytes from the file represented by fHandle into buffer
// Returns no of bytes read
//===============================================================
long ReadFile(struct FCB *fHandle, char *buffer, long noBytes)
{
   if (noBytes > fHandle->length)
   {
      return (0);
   }

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
      if ((fHandle->bufCursor == 512) && (bytesRead < noBytes))
      {
         if (fHandle->bufIsDirty)
         {
            WriteSector(
                  fHandle->filebuf,
                  ClusterToSector(fHandle->currentCluster)
                        + fHandle->sectorInCluster - 1);
         }
         if (fHandle->sectorInCluster++ == SectorsPerCluster)
         {
            fHandle->currentCluster = FAT[fHandle->currentCluster];
            fHandle->nextSector = ClusterToSector(fHandle->currentCluster);
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

   while (bytesWritten < noBytes)
   {
      while (fHandle->bufCursor < 512 && bytesWritten < noBytes)
      {
         fHandle->filebuf[fHandle->bufCursor] = buffer[bytesWritten];
         fHandle->bufIsDirty = 1;
         bytesWritten++;
         fHandle->length++;
         fHandle->bufCursor++;
      }

      // If we have written past the end of the buffer we need to flush the sector to the disk
      if ((fHandle->bufCursor == 512) && (bytesWritten < noBytes))
      {
         WriteSector(
               fHandle->filebuf,
               ClusterToSector(fHandle->currentCluster)
                     + fHandle->sectorInCluster - 1);
         if (fHandle->sectorInCluster++ > SectorsPerCluster)
         {
            // Allocate another cluster
            int freeCluster = FindFreeCluster();
            FAT[fHandle->currentCluster] = freeCluster;
            FAT[freeCluster] = 0xFFFF;
            fHandle->currentCluster = freeCluster;
            fHandle->nextSector = ClusterToSector(fHandle->currentCluster);
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
   	struct DirEntry *entry = (struct DirEntry *) FindFile(name, pid);

   	if (entry != 0)
   	{
    	entry->name[0] = 0xE5;
   		unsigned short int cluster = entry->startingCluster;
   		unsigned short int nextCluster;
   		while (cluster != 0xFFFF)
   		{
      		nextCluster = FAT[cluster];
      		FAT[cluster] = 0;
      		cluster = nextCluster;
   		}
   		SaveFAT();
   		SaveDir();
   		return (0);
   }
   else
   {
      return (1);
   }
}

//=============================
// The actual filesystem task
//=============================
void fsTaskCode(void)
{
   struct Message *FSMsg;
   struct MessagePort *tempPort;

   FSMsg = (struct Message *) AllocKMem(sizeof(struct Message));

   DiskBuffer = AllocUMem(512);
   int result;
   struct FCB *fcb;

   ((struct MessagePort *) FSPort)->waitingProc = (struct Task *) -1L;
   ((struct MessagePort *) FSPort)->msgQueue = 0;

   InitializeHD();

   while (1)
   {
      ReceiveMessage((struct MessagePort *) FSPort, FSMsg);
      switch (FSMsg->byte)
         {
      case CREATEFILE:
				 ;
         struct FCB *fcb = CreateFile((char *) FSMsg->quad, FSMsg->pid);
         tempPort = (struct MessagePort *) FSMsg->tempPort;
         if (!fcb)
         {
            FSMsg->quad = 0;
         }
         else
         {
            fcb->pid = FSMsg->pid;
            FSMsg->quad = (long) fcb;
         }
         SendMessage(tempPort, FSMsg);
         break;

      case OPENFILE:
         fcb = OpenFile((char *) FSMsg->quad, FSMsg->pid);
         tempPort = (struct MessagePort *) FSMsg->tempPort;
         if (!fcb)
         {
            FSMsg->quad = 0;
         }
         else
         {
            fcb->pid = FSMsg->pid;
            FSMsg->quad = (long) fcb;
         }
         SendMessage(tempPort, FSMsg);
         break;

      case CLOSEFILE:
         CloseFile((struct FCB *) FSMsg->quad);
         DeallocMem((void *) FSMsg->quad);
         tempPort = (struct MessagePort *) FSMsg->tempPort;
         SendMessage(tempPort, FSMsg);
         break;

      case READFILE:
         result = ReadFile((struct FCB *) FSMsg->quad, (char *) FSMsg->quad2,
               FSMsg->quad3);
         tempPort = (struct MessagePort *) FSMsg->tempPort;
         FSMsg->quad = result;
         SendMessage(tempPort, FSMsg);
         break;

      case WRITEFILE:
         result = WriteFile((struct FCB *) FSMsg->quad, (char *) FSMsg->quad2,
               FSMsg->quad3);
         tempPort = (struct MessagePort *) FSMsg->tempPort;
         FSMsg->quad = result;
         SendMessage(tempPort, FSMsg);
         break;

      case DELETEFILE:
         result = DeleteFile((char *) FSMsg->quad, FSMsg->pid);
         tempPort = (struct MessagePort *) FSMsg->tempPort;
         SendMessage(tempPort, FSMsg);
         break;

      case TESTFILE:
		 if (!strcmp(FSMsg->quad, "/"))
		 	result = 1;
		 else
         	result = (long) FindFile((char *) FSMsg->quad, FSMsg->pid);
         tempPort = (struct MessagePort *) FSMsg->tempPort;
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
         fcb = (struct FCB *) FSMsg->quad;
         info.Length = fcb->length;
         int count;
         for (count = 0; count < sizeof(struct FileInfo); count++)
            ((char *) FSMsg->quad2)[count] = ((char *) (&info))[count];
         tempPort = (struct MessagePort *) FSMsg->tempPort;
         SendMessage(tempPort, FSMsg);
         break;

      default:
         break;
         }
   }
}

