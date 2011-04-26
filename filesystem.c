#include "memory.h"
#include "kstructs.h"
#include "kernel.h"
#include "fat.h"
#include "filesystem.h"

extern struct Task *currentTask;

struct DirEntry *FindFreeDirEntry();


//===============================
// Convert a cluster to a sector.
//===============================
unsigned int ClusterToSector(int cluster)
{
    return((cluster - 2) * SectorsPerCluster + DataStart);
}


//=====================================
// Read in some parameters from the HD
//=====================================
void InitializeHD()
{
    struct MBR        *mbr;
    struct BootSector *bs;
    unsigned int      bootSector = 0;
    unsigned char     *FATbuffer;
    unsigned char     *RootDirBuffer;

    DiskBuffer = (unsigned char *)AllocUMem(512);

    ReadSector(DiskBuffer, 0L);
    mbr        = (struct MBR *)DiskBuffer;
    bootSector = (mbr->PT[0]).LBA;
    FirstFAT   = bootSector + 1;
    ReadSector(DiskBuffer, (long)bootSector);
    bs      = (struct BootSector *)DiskBuffer;
    RootDir = (bs->sectorsPerFat) * 2 + bs->reservedSectors + bootSector;
    RootDirectoryEntries = bs->rootEntries;
    DataStart            = (RootDirectoryEntries) / 16 + RootDir;
    SectorsPerCluster    = bs->sectorsPerCluster;
    BytesPerSector       = bs->bytesPerSector;
    FATLength            = (RootDir - FirstFAT) / 2;

    // Read FAT from disk to FAT buffer
    FATbuffer = AllocUMem(FATLength * BytesPerSector);
    int count;
    for (count = 0; count < FATLength; count++)
    {
        ReadSector(FATbuffer + (count * BytesPerSector), FirstFAT + count);
    }
    FAT = (unsigned short *)FATbuffer;

    // Read root directory from disk to buffer
    RootDirBuffer = AllocUMem(RootDirectoryEntries * 0x20);
    for (count = 0; count < (RootDirectoryEntries * 0x20) / BytesPerSector; count++)
    {
        ReadSector(RootDirBuffer + (count * BytesPerSector), RootDir + count);
    }
    RootDirectory = (struct DirEntry *)RootDirBuffer;
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
    i = 0;
    while (name[i] != '.' && i < 8 && name[i] != 0)
    {
        dirname[j++] = name[i++];
    }
    if (name[i] == 0) return(0);
    if ((i == 8) && (name[i] != '.'))
    {
        return(1);
    }
    j = 8;
    i++;
    while (name[i] != 0 && j < 11)
    {
        dirname[j++] = name[i++];
    }
    return(0);
}


//===============================================================================
// Find the root directory entry for the file whose name is pointed to by "name".
// Return the address of the directory entry, or 0 on failure.
//===============================================================================
long FindFile(char name[13])
{
    short int       done = 1;
    struct DirEntry *entries;

    char dirname[11];

    if (NameToDirName(name, dirname))
    {
        return(0);
    }

    short int n;
    for (n = 0; n < RootDirectoryEntries; n++)
    {
        done = 0;
        short int i;
        for (i = 0; i < 11; i++)
        {
            if (RootDirectory[n].name[i] != dirname[i])
            {
                done = 1;
            }
        }
        if (done == 0)
        {
            return((long)&RootDirectory[n]);
        }
    }
    return(0);
}


//=================================
// Finds next free cluster on disk
//=================================
int FindFreeCluster()
{
    int count = 0;

    while (FAT[count] != 0)
    {
        count++;
    }
    return(count);
}


//=========================================
// Find a free directory entry
// Return a pointer to the directory entry
//        or 0 on failure
//=========================================
struct DirEntry *FindFreeDirEntry()
{
    int             count = 0;
    struct DirEntry *entries;

    while (RootDirectory[count].name[0] != 0xE5)
    {
        if (RootDirectory[count].name[0] == 0)
        {
            return(&RootDirectory[count]);
        }
        count++;
        if (count == RootDirectoryEntries)
        {
            return(0);
        }
    }
    return(&RootDirectory[count]);
}


//==================================
// Save the FAT buffer back to disk
//==================================
void SaveFAT()
{
    char *FATbuffer = (char *)FAT;
    int  count;

    for (count = 0; count < FATLength; count++)
    {
        WriteSector(FATbuffer + (count * BytesPerSector), FirstFAT + count);
    }
}


//========================================
// Save the Directory buffer back to disk
//========================================
void SaveDir()
{
    int  count;
    char *RootDirBuffer = (char *)RootDirectory;

    for (count = 0; count < (RootDirectoryEntries * 0x20) / BytesPerSector; count++)
    {
        WriteSector(RootDirBuffer + (count * BytesPerSector), RootDir + count);
    }
}


//======================================
// Create a new file.
// Returns 0 on success
//       1 if file cannot be created
//======================================
int CreateFile(char *name, struct FCB *fHandle)
{
    if (FindFile(name))
    {
        return(1);
    }
    struct DirEntry *entry;
    if ((entry = FindFreeDirEntry()) == 0)
    {
        return(1);
    }
    int count;

    // Zero directory entry
    for (count = 0; count < sizeof(struct DirEntry); count++)
    {
        ((unsigned char *)entry)[count] = (unsigned char)0;
    }

    // Fill in a few details
    if (NameToDirName(name, entry->name))
    {
        return(1);
    }
    entry->startingCluster = FindFreeCluster();
    entry->attribute       = 0x20;

    // Mark the cluster as in use
    FAT[entry->startingCluster] = 0xFFFF;

    SaveFAT();
    SaveDir();
    fHandle->directory       = entry;
    fHandle->currentCluster  = entry->startingCluster;
    fHandle->startSector     = ClusterToSector(entry->startingCluster);
    fHandle->startCluster    = entry->startingCluster;
    fHandle->fileCursor      = fHandle->bufCursor = fHandle->bufIsDirty = 0;
    fHandle->length          = entry->fileSize;
    fHandle->filebuf         = (char *)AllocUMem(512);
    fHandle->sectorInCluster = 1;
    return(0);
}


//===================================
// Open a file.
// Returns 0 on success
//       1 if file does not exist
//===================================
int OpenFile(char name[11], struct FCB *fHandle)
{
    struct DirEntry *entry = (struct DirEntry *)FindFile(name);

    if (entry != 0)
    {
        fHandle->directory      = entry;
        fHandle->currentCluster = entry->startingCluster;
        fHandle->startSector    = ClusterToSector(entry->startingCluster);
        fHandle->startCluster   = entry->startingCluster;
        fHandle->fileCursor     = fHandle->bufCursor = fHandle->bufIsDirty = 0;
        fHandle->length         = entry->fileSize;
        fHandle->filebuf        = (char *)AllocUMem(512);
        ReadSector(fHandle->filebuf, fHandle->startSector);
        fHandle->nextSector      = fHandle->startSector + 1;
        fHandle->sectorInCluster = 1;
        return(0);
    }
    else
    {
        return(1);
    }
}


//============================================
// Close a file and release all its resources
//============================================
void CloseFile(struct FCB *fHandle)
{
    struct clusterListEntry *clusters, *nextcluster;

    if (fHandle->bufIsDirty)
    {
        WriteSector(fHandle->filebuf, ClusterToSector(fHandle->currentCluster) + fHandle->sectorInCluster - 1);
    }
    fHandle->directory->fileSize = fHandle->length;
    SaveDir();
    DeallocMem((struct MemStruct *)fHandle->filebuf);
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
        return(0);
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
                WriteSector(fHandle->filebuf, ClusterToSector(fHandle->currentCluster) + fHandle->sectorInCluster - 1);
            }
            if (fHandle->sectorInCluster++ == SectorsPerCluster)
            {
                fHandle->currentCluster  = FAT[fHandle->currentCluster];
                fHandle->nextSector      = ClusterToSector(fHandle->currentCluster);
                fHandle->sectorInCluster = 1;
            }
            ReadSector(fHandle->filebuf, fHandle->nextSector);
            fHandle->bufCursor = 0;
            fHandle->nextSector++;
        }
    }
    return(bytesRead);
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

        // If we have read past the end of the buffer we need to load the
        // next sector into the buffer.
        if ((fHandle->bufCursor == 512) && (bytesWritten < noBytes))
        {
            WriteSector(fHandle->filebuf, ClusterToSector(fHandle->currentCluster) + fHandle->sectorInCluster - 1);
            if (fHandle->sectorInCluster++ > SectorsPerCluster)
            {
                // Allocate another cluster
                int freeCluster = FindFreeCluster();
                FAT[fHandle->currentCluster] = freeCluster;
                FAT[freeCluster]             = 0xFFFF;
                fHandle->currentCluster      = freeCluster;
                fHandle->nextSector          = ClusterToSector(fHandle->currentCluster);
                fHandle->sectorInCluster     = 1;
            }
            fHandle->bufCursor = 0;
            fHandle->nextSector++;
        }
    }
    return(bytesWritten);
}

int DeleteFile(struct FCB *fHandle)
{
    fHandle->directory->name[0] = 0xE5;
    unsigned short int cluster = fHandle->directory->startingCluster;
    unsigned short int nextCluster;
    while (cluster != 0xFFFF)
    {
        nextCluster  = FAT[cluster];
        FAT[cluster] = 0;
        cluster      = nextCluster;
    }
    SaveFAT();
    SaveDir();
    return(0);
}


//=============================
// The actual filesystem task
//=============================
void fsTaskCode()
{
    struct Message     *FSMsg;
    struct MessagePort *tempPort;

    FSMsg = (struct Message *)AllocKMem(sizeof(struct Message));

    DiskBuffer = AllocUMem(512);
    int result;
    struct FCB *fcb;

    ((struct MessagePort *)FSPort)->waitingProc = (struct Task *)-1L;
    ((struct MessagePort *)FSPort)->msgQueue    = 0;

    InitializeHD();

    while (1)
    {
        ReceiveMessage((struct MessagePort *)FSPort, FSMsg);
        switch (FSMsg->byte)
        {
        case CREATEFILE:
            fcb      = (struct FCB *)AllocUMem(sizeof(struct FCB));
            result   = CreateFile((char *)FSMsg->quad, fcb);
            tempPort = (struct MessagePort *)FSMsg->tempPort;
            if (result)
            {
                FSMsg->quad = 0;
            }
            else
            {
                fcb->pid    = FSMsg->pid;
                FSMsg->quad = (long)fcb;
            }
            SendMessage(tempPort, FSMsg);
            break;

        case OPENFILE:
            fcb      = (struct FCB *)AllocUMem(sizeof(struct FCB));
            result   = OpenFile((char *)FSMsg->quad, fcb);
            tempPort = (struct MessagePort *)FSMsg->tempPort;
            if (result)
            {
                FSMsg->quad = 0;
            }
            else
            {
                fcb->pid    = FSMsg->pid;
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
            result      = ReadFile((struct FCB *)FSMsg->quad, (char *)FSMsg->quad2, FSMsg->quad3);
            tempPort    = (struct MessagePort *)FSMsg->tempPort;
            FSMsg->quad = result;
            SendMessage(tempPort, FSMsg);
            break;

        case WRITEFILE:
            result      = WriteFile((struct FCB *)FSMsg->quad, (char *)FSMsg->quad2, FSMsg->quad3);
            tempPort    = (struct MessagePort *)FSMsg->tempPort;
            FSMsg->quad = result;
            SendMessage(tempPort, FSMsg);
            break;

        case DELETEFILE:
            result = DeleteFile((struct FCB *)FSMsg->quad);
            DeallocMem((void *)FSMsg->quad);
            tempPort = (struct MessagePort *)FSMsg->tempPort;
            SendMessage(tempPort, FSMsg);
            break;

        case GETPID:
            FSMsg->quad = currentTask->pid;
            SendMessage(tempPort, FSMsg);
            break;

        case GETDIRENTRY:
            ;
            struct DirEntry * temp = (struct DirEntry *)&RootDirectory[FSMsg->quad];
            int count;
            for (count = 0; count < sizeof(struct DirEntry); count++)
                ((char *)FSMsg->quad2)[count] = ((char *)temp)[count];
            tempPort = (struct MessagePort *)FSMsg->tempPort;
            SendMessage(tempPort, FSMsg);
            break;

        default:
            break;
        }
    }
}

int debug()
{
}
