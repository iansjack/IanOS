#include <kernel.h>
#include <filesystem.h>

struct DirEntry *
FindFile(unsigned char *name);
struct DirEntry *
FindFileDirectorySector(unsigned char *filename, struct vDirNode *directory,
    int *sector, int *entryOffset);
void
CreateVDirectory(void);
unsigned char *
DirNameToName(unsigned char dirname[11]);
struct vDirNode *vDirectory;
struct BTreeNode *sectorBuffers;
int buffersRead;
unsigned short *currentFATBuffer;
int currentFATSector;
unsigned char *DiskBuffer;
unsigned short *FAT;

unsigned long FirstFAT;
unsigned long RootDir;
unsigned long DataStart;
unsigned long FATLength;
unsigned short BytesPerSector;
unsigned char SectorsPerCluster;
unsigned short RootDirectoryEntries;

extern struct MessagePort *FSPort;
extern long sec, min, hour, day, month, year;

//===============================
// Convert a cluster to a sector
//===============================
unsigned int
ClusterToSector(int cluster)
{
  return ((cluster - 2) * SectorsPerCluster + DataStart);
}

//===============================
// Convert a sector to a cluster
//===============================
unsigned int
SectorToCluster(int sector)
{
  return (sector - DataStart) / SectorsPerCluster + 2;
}

//===============================================
// Convert the current time to a filetime format
//===============================================
short
ClockToFileTime()
{
  short retval = 0;
  retval += sec;
  retval += min << 5;
  retval += hour << 11;
  return retval;
}

//===============================================
// Convert the current date to a filedate format
//===============================================
short
ClockToFileDate()
{
  short retval = 0;
  retval += day;
  retval += month << 5;
  retval += (year + 20) << 9;
  return retval;
}

//===================================================================
// Read a disk sector. If the sector is already in the buffer a
// pointer to it is returned. Otherwise a new buffer entry is
// created and a pointer to that buffer returned.
//===================================================================
unsigned char *
ReadSector(unsigned int sector)
{
  if (!sectorBuffers)
    {
      void *sectorBuffer = AllocUMem(512);
      ReadPSector(sectorBuffer, sector);
      sectorBuffers = CreateBTreeNode(sector, sectorBuffer);
      return sectorBuffer;
    }
  else
    {
      if (buffersRead >= 10)
        {
          buffersRead = 0;
          sectorBuffers = BalanceBTree(sectorBuffers);
        }
      struct BTreeNode *node = FindBTreeNode(sectorBuffers, sector);
      if (!node)
        {
          void *sectorBuffer = AllocUMem(512);
          ReadPSector(sectorBuffer, sector);
          AddBTreeNode(sectorBuffers, sector, sectorBuffer);
          buffersRead++;
          return sectorBuffer;
        }
      else
        return node->data;
    }
}

//===================================================
// Write a sector, which just marks it as dirty
//===================================================
void
WriteSector(unsigned int sector)
{
  struct BTreeNode *node = FindBTreeNode(sectorBuffers, sector);
  node->isDirty = 1;
}

//===================================================
// Flush any dirty sectors to disk
//===================================================
void
FlushSectorBuffers(struct BTreeNode *node)
{
  if (node->isDirty)
    {
      WritePSector(node->data, node->key);
      node->isDirty = 0;
    }
  if (node->greater)
    FlushSectorBuffers(node->greater);
  if (node->lesser)
    FlushSectorBuffers(node->lesser);
}

//=====================================
// Read in some parameters from the HD
//=====================================
void
InitializeHD(void)
{
  struct MBR *mbr;
  struct BootSector *bs;
  unsigned int bootSector = 0;

  DiskBuffer = ReadSector(0);
  mbr = (struct MBR *) DiskBuffer;
  bootSector = (mbr->PT[0]).LBA;
  DiskBuffer = ReadSector(bootSector);
  bs = (struct BootSector *) DiskBuffer;
  RootDir = (bs->sectorsPerFat) * 2 + bs->reservedSectors + bootSector;
  RootDirectoryEntries = bs->rootEntries;
  DataStart = (RootDirectoryEntries) / 16 + RootDir;
  SectorsPerCluster = bs->sectorsPerCluster;
  BytesPerSector = bs->bytesPerSector;
  FirstFAT = bootSector + bs->reservedSectors;
  FATLength = (RootDir - FirstFAT) / 2;

  currentFATSector = FirstFAT;
  currentFATBuffer = (unsigned short *) ReadSector(currentFATSector);

  CreateVDirectory();
}

//===============================================================
// Get the FAT entry for cluster
//===============================================================
unsigned short
GetFATEntry(unsigned short cluster)
{
  int sector = FirstFAT + (2 * cluster / BytesPerSector);
  if (sector != currentFATSector)
    {
      currentFATBuffer = (unsigned short *) ReadSector(sector);
      currentFATSector = sector;
    }
  return currentFATBuffer[cluster % (BytesPerSector / 2)];
}

//===============================================================
// Set the FAT entry for cluster to value
//===============================================================
void
PutFATEntry(unsigned short cluster, unsigned short value)
{
  int sector = FirstFAT + (2 * cluster / BytesPerSector);
  if (sector != currentFATSector)
    {
      currentFATBuffer = (unsigned short *) ReadSector(sector);
      currentFATSector = sector;
    }
  currentFATBuffer[cluster % (BytesPerSector / 2)] = value;
  WriteSector(currentFATSector);
}

//=======================================================
// Create the subtree of vDirNodes for the current node  *** Some work needed here to read all sectors ***
//=======================================================
void
GetVDirEntries(struct vDirNode *currentDir)
{
  int i;

  struct DirEntry *dirBuffer = (struct DirEntry *) ReadSector(
      currentDir->startSector);
  for (i = 0; i < 16; i++)
    {
      if (dirBuffer[i].name[0] != 0xE5 && dirBuffer[i].name[0] != 0)
        {
          if (dirBuffer[i].attribute & 0x10)
            {
              // It's a directory!!!
              struct vDirNode *temp = AllocUMem(sizeof(struct vDirNode));
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
}

//============================================================
// Scan the hard disk and create the vDirectory tree
//============================================================
void
CreateVDirectory(void)
{
  struct vDirNode *currentDir;

  // Create the entry for the root directory
  vDirectory = AllocUMem(sizeof(struct vDirNode));
  vDirectory->name = AllocUMem(2);
  strcpy(vDirectory->name, "/");
  vDirectory->startSector = RootDir;
  vDirectory->parent = 0;
  vDirectory->nextSibling = 0;
  vDirectory->firstChild = 0;
  currentDir = vDirectory;

  // Now fill in details for all directories in the root directory
  // This will recursively scan the whole directory tree
  GetVDirEntries(currentDir);
}

//============================================================
// Given a full path return a pointer to the name part
// Note that this function does not allocate any memory
//============================================================
unsigned char *
GetFilename(unsigned char *fullpath)
{
  int i = strlen(fullpath);
  while (fullpath[i] != '/')
    i--;
  return (unsigned char *) fullpath + i + 1;
}

//==========================================================
// Convert filename as a string to the Directory Entry form
//      Returns 0 on success
//              1 on failure
//==========================================================
int
NameToDirName(char *name, char *dirname)
{
  short int i = 0;
  short int j = 0;

  for (i = 0; i < 11; i++)
    dirname[i] = ' ';
  if (name[0] == '.' && name[1] == '.' && (name[3] == 0 || name[3] == ' '))
    {
      dirname[0] = dirname[1] = '.';
      return 0;
    }
  i = 0;
  while (name[i] != '.' && i < 8 && name[i] != 0)
    dirname[j++] = name[i++];
  if (name[i] == 0)
    return (0);
  if ((i == 8) && (name[i] != '.'))
    return (1);
  j = 8;
  i++;
  while (name[i] != 0 && j < 11)
    dirname[j++] = name[i++];
  return (0);
}

//==========================================================
// Convert filename as Directory Entry form to a string
//      Returns a pointer to the string
//==========================================================
unsigned char *
DirNameToName(unsigned char dirname[11])
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
// Find virtual directory node of a file. If not found return 0.
//============================================================================
struct vDirNode *
FindDirectory(unsigned char *fullpath)
{
  if (fullpath[0] == '/')
    fullpath++;

  struct vDirNode *temp = vDirectory;
  unsigned char *restOfString;
  while (1)
    {
      restOfString = strchr(fullpath, '/');
      if (!restOfString) // Found it
        return temp;
      restOfString[0] = 0;
      restOfString++;
      temp = temp->firstChild;
      while (strcmp(temp->name, fullpath))
        {
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
struct DirEntry *
FindFile(unsigned char *name)
{
  struct DirEntry *buffer;
  struct DirEntry *entry;
  int entryOffset = 0;
  int sector = 0;

  unsigned char *tempName = AllocUMem(strlen(name) + 1);
  strcpy(tempName, name);
  unsigned char *filename = GetFilename(name);
  struct vDirNode *directory = FindDirectory(tempName);

  buffer = FindFileDirectorySector(filename, directory, &sector, &entryOffset);
  DeallocUMem(tempName);
  if (!buffer)
    return 0;
  return buffer + entryOffset;
}

//======================================================================================================
// Find the directory details of the file name.
// Returns the the buffer of the sector containing the directory entry, or 0 if not found.
// Sets sector to the sector number of the entry, entryOffset to the offset to the entry.
//======================================================================================================
struct DirEntry *
FindFileDirectorySector(unsigned char *filename, struct vDirNode *directory,
    int *sector, int *entryOffset)
{
  *sector = 0;
  unsigned char *dirName = 0;
  struct DirEntry *buffer = 0;
  struct DirSects dirSector;

  *sector = FindFirstDirectorySector(directory, &dirSector);
  if (*sector)
    {
      buffer = (struct DirEntry *) ReadSector(*sector);
      struct DirEntry *temp = buffer;

      while (*sector)
        {
          int entriesInSect = BytesPerSector / sizeof(struct DirEntry);
          int entryno = 0;
          while (entryno < entriesInSect)
            {
              // Have we reached the end of the directory entries?
              if (temp->name[0] == 0)
                return 0;

              // Is this an erased entry - in which case we're not interested in it
              if (temp->name[0] == 0xE5)
                break;

              // So, is it the entry for this filename?
              dirName = DirNameToName(temp->name);
              if (!strcmp(dirName, filename))
                {
                  // Found it!
                  DeallocUMem(dirName);
                  *entryOffset = entryno;
                  return buffer;
                }

              // No. Look at next entry
              DeallocUMem(dirName);
              entryno++;
              temp++;
            }

          // Does this directory have another sector? If so fetch it and keep looking.
          *sector = FindNextDirectorySector(&dirSector);
          entryno = 0;
        }
      if (!*sector)
        buffer = 0;
    }
  return (buffer);
}

//=================================
// Finds next free cluster on disk
//=================================
int
FindFreeCluster(void)
{
  int count = 0;

  while (GetFATEntry(++count) != 0)
    ;
  return (count);
}

//======================================================================
// Find an empty directory slot.
// Returns a pointer to the zero-filled entry, or 0 on failure
// Buffer will contain the sector of the directory containing the entry
//======================================================================
struct DirEntry *
FindEmptyDirectorySlot(struct DirEntry *directory, struct vDirNode *dir)
{
  if (dir)
    {
      struct DirSects dirSect;
      int sector = FindFirstDirectorySector(dir, &dirSect);
      directory = (struct DirEntry *) ReadSector(sector);
      struct DirEntry *entry = directory;
      int count;

      // Find a free entry
      while (!(entry->name[0] == 0xE5 || entry->name[0] == 0))
        {
          entry++;
          if ((unsigned char *) entry == (unsigned char *) directory + 512)
            {
              sector = FindNextDirectorySector(&dirSect); // Need some code here to deal with the fact that there are no free entries
              directory = (struct DirEntry *) ReadSector(sector);
            }
        }

      // Zero directory entry
      for (count = 0; count < sizeof(struct DirEntry); count++)
        ((unsigned char *) entry)[count] = (unsigned char) 0;
      return entry;
    }
  return 0;
}

//======================================
// Create a new file.
// Returns 1 on success
//       0 if file cannot be created
//======================================
struct FCB *
CreateFile(unsigned char *name, unsigned short pid)
{
  // If the file already exist, exit
  if (FindFile(name))
    return (0);

  struct vDirNode *dir = FindDirectory(name);
  struct DirEntry *directory;
  unsigned char *filename = GetFilename(name);
  struct DirEntry *entry = FindEmptyDirectorySlot(directory, dir);

  if (entry)
    {
      // Fill in a few details
      if (NameToDirName(filename, entry->name))
        return (0);

      entry->startingCluster = FindFreeCluster();
      entry->attribute = 0x20;
      entry->modifiedDate = ClockToFileDate();
      entry->modifiedTime = ClockToFileTime();

      // Mark the cluster as in use
      PutFATEntry(entry->startingCluster, 0xFFFF);
      WriteSector(dir->startSector);

      // Flush the written buffers to disk
      FlushSectorBuffers(sectorBuffers);
      struct FCB *fHandle = (struct FCB *) AllocKMem(sizeof(struct FCB));
      fHandle->dirEntry = entry;
      fHandle->dir = dir;
      fHandle->currentCluster = entry->startingCluster;
      fHandle->startSector = ClusterToSector(entry->startingCluster);
      fHandle->startCluster = entry->startingCluster;
      fHandle->fileCursor = fHandle->bufCursor = fHandle->bufIsDirty = 0;
      fHandle->length = 0;
      fHandle->filebuf = ReadSector(fHandle->startSector);
      fHandle->sectorInCluster = 1;
      return (fHandle);
    }
  else
    return (0);
}

//===================================================================
// Open a file.
// Returns fHandle on success.
//       0 if file does not exist
//===================================================================
struct FCB *
OpenFile(unsigned char *name, unsigned short pid)
{
  struct FCB *fHandle = AllocKMem(sizeof(struct FCB));

  // Root directory isn't a normal file
  if (!strcmp(name, "/"))
    {
      fHandle->deviceType = DIR;
      fHandle->dirEntry = 0;
      fHandle->dir = 0;
      fHandle->currentCluster = 0;
      fHandle->startSector = RootDir;
      fHandle->startCluster = 0;
      fHandle->fileCursor = fHandle->bufCursor = fHandle->bufIsDirty = 0;
      fHandle->length = 512;
      fHandle->filebuf = ReadSector(fHandle->startSector);
      fHandle->nextSector = fHandle->startSector + 1;
      fHandle->sectorInCluster = 1;
      return (fHandle);
    }
  else
    {
      struct DirEntry *entry = (struct DirEntry *) FindFile(name);

      if (entry != 0)
        {
          fHandle->dirEntry = entry;
          fHandle->dir = FindDirectory(name);
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
          fHandle->filebuf = ReadSector(fHandle->startSector);
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
void
CloseFile(struct FCB *fHandle)
{
  int offset;
  if (fHandle->bufIsDirty)
    {
      WriteSector(
          ClusterToSector(fHandle->currentCluster) + fHandle->sectorInCluster
              - 1);
    }
  if (fHandle->dirEntry) // If no dirEntry it must be the root directory.
    {
      unsigned char *filename = DirNameToName(fHandle->dirEntry->name);

      if (fHandle->deviceType == FILE && fHandle->bufIsDirty)
        {
          int sector = 0;
          struct DirEntry *buffer;
          buffer = FindFileDirectorySector(filename, fHandle->dir, &sector,
              &offset);
          struct DirEntry *temp = buffer + offset;
          if (fHandle->deviceType == FILE)
            {
              temp->fileSize = fHandle->length;
              temp->modifiedDate = ClockToFileDate();
              temp->modifiedTime = ClockToFileTime();
              WriteSector(sector);
            }
        }
      DeallocUMem(filename);
    }
  DeallocMem(fHandle);
  FlushSectorBuffers(sectorBuffers);
}

//=======================================================================
// Find the first sector of a directory
//=======================================================================
int
FindFirstDirectorySector(struct vDirNode *directory, struct DirSects *dirSect)
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
int
FindNextDirectorySector(struct DirSects *dirSect)
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
          if (dirSect->sector
              > RootDirectoryEntries * sizeof(struct DirEntry) / BytesPerSector)
            dirSect->sector = 0;
        }
      else // it's a normal directory so get next cluster
        {
          dirSect->cluster = GetFATEntry(dirSect->cluster);
          if (dirSect->cluster != 0xFFFF)
            {
              dirSect->sector = ClusterToSector(dirSect->cluster);
              dirSect->sectorInCluster = 1;
            }
          else
            dirSect->sector = 0;
        }
    }
  return dirSect->sector;
}

//===============================================================
// Read noBytes from the file represented by fHandle into buffer
// Returns no of bytes read
//===============================================================
long
ReadFile(struct FCB *fHandle, char *buffer, long noBytes)
{
  long bytesRead = 0;

  while (bytesRead < noBytes && fHandle->bufCursor < fHandle->length)
    {
      while (fHandle->bufCursor < 512 && bytesRead < noBytes
          && fHandle->bufCursor < fHandle->length)
        {
          buffer[bytesRead] = fHandle->filebuf[fHandle->bufCursor];
          bytesRead++;
          fHandle->bufCursor++;
        }
      if (fHandle->bufCursor == fHandle->length)
        break;

      // If we have read past the end of the buffer we need to load the
      // next sector into the buffer.
      if ((fHandle->bufCursor == 512) && (bytesRead < noBytes))
        {
          if (fHandle->bufIsDirty)
            {
              WriteSector(
                  ClusterToSector(fHandle->currentCluster)
                      + fHandle->sectorInCluster - 1);
              FlushSectorBuffers(sectorBuffers);
            }
          if (fHandle->sectorInCluster++ == SectorsPerCluster)
            {
              fHandle->currentCluster = GetFATEntry(fHandle->currentCluster);
              fHandle->nextSector = ClusterToSector(fHandle->currentCluster);
              fHandle->sectorInCluster = 1;
            }
          fHandle->filebuf = ReadSector(fHandle->nextSector);
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
long
WriteFile(struct FCB *fHandle, char *buffer, long noBytes)
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
              ClusterToSector(fHandle->currentCluster)
                  + fHandle->sectorInCluster - 1);
          if (fHandle->sectorInCluster++ > SectorsPerCluster)
            {
              // Allocate another cluster
              int freeCluster = FindFreeCluster();
              PutFATEntry(fHandle->currentCluster, freeCluster);
              PutFATEntry(freeCluster, 0xFFFF);
              fHandle->currentCluster = freeCluster;
              fHandle->nextSector = ClusterToSector(fHandle->currentCluster);
              fHandle->sectorInCluster = 1;
            }
          fHandle->bufCursor = 0;
          fHandle->nextSector++;
        }
    }
  FlushSectorBuffers(sectorBuffers);
  return (bytesWritten);
}

//===================================
// Delete a file.
// Returns 0 on success
//       1 if file does not exist
//===================================
int
DeleteFile(unsigned char *name, unsigned short pid)
{
  struct DirEntry *entry = (struct DirEntry *) FindFile(name);

  if (entry != 0)
    {
      // We now know the file exists
      // Erase the directory entry
      entry->name[0] = 0xE5;
      // Zero the FAT entries for the file
      unsigned short int cluster = entry->startingCluster;
      unsigned short int nextCluster;
      while (GetFATEntry(cluster) != 0xFFFF)
        {
          nextCluster = GetFATEntry(cluster);
          PutFATEntry(cluster, 0);
          cluster = nextCluster;
        }
      PutFATEntry(cluster, 0);

      // Write the changed FAT and directory sectors back to disk
      FlushSectorBuffers(sectorBuffers);
      return (0);
    }
  else
    return (1);
}

long
CreateDir(unsigned char *name, unsigned short pid)
{
  // If the directory already exist, exit
  if (FindFile(name))
    return (-1);

  struct vDirNode *dir = FindDirectory(name);
  struct DirEntry *directory;
  unsigned char *filename = GetFilename(name);
  struct DirEntry *entry = FindEmptyDirectorySlot(directory, dir);

  if (entry)
    {
      struct DirSects dirSect;
      int sector = FindFirstDirectorySector(dir, &dirSect);

      // Fill in a few details
      if (NameToDirName(filename, entry->name))
        return (-1);

      entry->startingCluster = FindFreeCluster();
      entry->attribute = 0x10;

      long startingCluster = entry->startingCluster;
      // Mark the cluster as in use
      PutFATEntry(entry->startingCluster, 0xFFFF);

      WriteSector(dir->startSector);

      // Create the . and .. entries and blank the new cluster
      unsigned char *udirectory = (unsigned char *) directory;
      sector = ClusterToSector(startingCluster);
      directory = (struct DirEntry *) ReadSector(sector);

      int count;
      for (count = 0; count < BytesPerSector; count++)
        udirectory[count] = 0;
      entry = directory;
      for (count = 0; count < 11; count++)
        entry->name[count] = ' ';
      entry->name[0] = '.';
      entry->attribute = 0x10;
      entry->startingCluster = SectorToCluster(dir->startSector);
      entry++;
      for (count = 0; count < 11; count++)
        entry->name[count] = ' ';
      entry->name[0] = entry->name[1] = '.';
      entry->attribute = 0x10;
      entry->startingCluster = startingCluster;
      entry->modifiedDate = ClockToFileDate();
      entry->modifiedTime = ClockToFileTime();
      WriteSector(sector);
      for (count = 0; count < SectorsPerCluster - 1; count++)
        {
          sector++;
          directory = (struct DirEntry *) ReadSector(sector);
          int count2;
          for (count2 = 0; count2 < BytesPerSector; count2++)
            udirectory[count2] = 0;
          WriteSector(sector);
        }
      FlushSectorBuffers(sectorBuffers);

      // Make entry in vDirectory
      struct vDirNode *newNode = AllocUMem(sizeof(struct vDirNode));
      newNode->firstChild = 0;
      newNode->parent = dir;
      newNode->nextSibling = 0;
      newNode->name = AllocUMem(strlen(filename) + 1);
      if (!dir->firstChild)
        dir->firstChild = newNode;
      else
        {
          dir = dir->firstChild;
          while (dir->nextSibling)
            dir = dir->nextSibling;
          dir->nextSibling = newNode;
        }
      return (0);
    }
  else
    return (-1);
}

int
Seek(struct FCB *fHandle, int offset, int whence)
{
  switch (whence)
    {
  case SEEK_SET:
    fHandle->fileCursor = offset;
    break;

  case SEEK_CUR:
    fHandle->fileCursor += offset;
    break;

  case SEEK_END:
    fHandle->fileCursor = fHandle->length + offset;
    break;

  default:
    return -1;
    }

  if (fHandle->fileCursor < 0)
    {
      fHandle->fileCursor = 0;
      offset = 0;
    }

  if (fHandle->fileCursor > fHandle->length)
    {
      fHandle->fileCursor = fHandle->length;
      offset = fHandle->length;
    }

  fHandle->bufCursor = fHandle->fileCursor;
  fHandle->currentCluster = fHandle->startCluster;
  fHandle->sectorInCluster = 1;

  while (fHandle->bufCursor > BytesPerSector)
    {
      fHandle->bufCursor -= BytesPerSector;
      if (fHandle->bufIsDirty)
        {
          WriteSector(
              ClusterToSector(fHandle->currentCluster)
                  + fHandle->sectorInCluster - 1);
          FlushSectorBuffers(sectorBuffers);
        }
      if (fHandle->sectorInCluster++ == SectorsPerCluster)
        {
          fHandle->currentCluster = GetFATEntry(fHandle->currentCluster);
          fHandle->nextSector = ClusterToSector(fHandle->currentCluster);
          fHandle->sectorInCluster = 1;
        }
      fHandle->filebuf = ReadSector(fHandle->nextSector);
      fHandle->nextSector++;
    }
  return offset;
}

//=============================
// The actual filesystem task
//=============================
void
fsTaskCode(void)
{
  kprintf(3, 0, "Starting Filesystem Task");

  FSPort = AllocMessagePort();

  struct Message *FSMsg;
  struct MessagePort *tempPort;

  FSMsg = (struct Message *) ALLOCMSG;

  int result;
  struct FCB *fcb;

  FSPort->waitingProc = (struct Task *) -1L;
  FSPort->msgQueue = 0;

  sectorBuffers = 0;
  buffersRead = 0;
  InitializeHD();

  while (1)
    {
      ReceiveMessage(FSPort, FSMsg);
      tempPort = FSMsg->tempPort;
      switch (FSMsg->byte)
        {
      case CREATEFILE:
        ;
        struct FCB *fcb = CreateFile((char *) FSMsg->quad, FSMsg->pid);
        if (!fcb)
          FSMsg->quad = 0;
        else
          {
            fcb->pid = FSMsg->pid;
            FSMsg->quad = (long) fcb;
          }
        SendMessage(tempPort, FSMsg);
        break;

      case OPENFILE:
        fcb = OpenFile((unsigned char *) FSMsg->quad, FSMsg->pid);
        if (!fcb)
          FSMsg->quad = 0;
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
        SendMessage(tempPort, FSMsg);
        break;

      case READFILE:
        result = ReadFile((struct FCB *) FSMsg->quad, (char *) FSMsg->quad2,
            FSMsg->quad3);
        FSMsg->quad = result;
        SendMessage(tempPort, FSMsg);
        break;

      case WRITEFILE:
        result = WriteFile((struct FCB *) FSMsg->quad, (char *) FSMsg->quad2,
            FSMsg->quad3);
        FSMsg->quad = result;
        SendMessage(tempPort, FSMsg);
        break;

      case DELETEFILE:
        result = DeleteFile((char *) FSMsg->quad, FSMsg->pid);
        SendMessage(tempPort, FSMsg);
        break;

      case TESTFILE:
        if (!strcmp(FSMsg->quad, "/"))
          result = 1;
        else
          {
            result = (long) FindFile((char *) FSMsg->quad);
            if (result)
              result = 1;
          }

        FSMsg->quad = result;
        SendMessage(tempPort, FSMsg);
        break;

      case GETFILEINFO:
        ;
        struct FileInfo info;
        fcb = (struct FCB *) FSMsg->quad;
        info.Length = fcb->length;
        info.modifiedDate = fcb->dirEntry->modifiedDate;
        info.modifiedTime = fcb->dirEntry->modifiedTime;
        copyMem((char *) (&info), (char *) FSMsg->quad2,
            sizeof(struct FileInfo));
        SendMessage(tempPort, FSMsg);
        break;

      case CREATEDIR:
        result = CreateDir((char *) FSMsg->quad, FSMsg->pid);
        FSMsg->quad = result;
        SendMessage(tempPort, FSMsg);
        break;

      case SEEK:
        result = Seek((struct FCB *) FSMsg->quad, FSMsg->quad2, FSMsg->quad3);
        FSMsg->quad = result;
        SendMessage(tempPort, FSMsg);
        break;

      default:
        break;
        }
    }
}
