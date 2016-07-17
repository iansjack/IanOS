#include <kernel.h>
//#include <linux/types.h>
#include <errno.h>
#include "ext2_fs.h"
#include "blocks.h"

struct FCB *OpenFile(char *path);
long ReadFile(struct FCB *fcb, char *buffer, long noBytes);
long WriteFile(struct FCB *fcb, char *buffer, long noBytes);
long CloseFile(struct FCB *fcb);
long Seek(struct FCB *fcb, int offset, int whence);

extern int block_size;
extern struct ext2_super_block sb;
extern struct ext2_group_desc *group_descriptors;
extern struct MessagePort *FSPort;
extern long unixtime;

long ReadFromFile(struct FCB *, char *, long);
long WriteToFile(struct FCB *, char *, long);

//============================================================
// Open the file represented by inode and assign an FCB to it
//============================================================
struct FCB *OpenFileByInodeNumber(u_int32_t inode)
{
	struct FCB *fcb = AllocKMem(sizeof(struct FCB));
	fcb->inode = AllocKMem(sizeof(struct ext2_inode));
	fcb->inodeNumber = inode;
	GetINode(inode, fcb->inode);
	fcb->fileCursor = 0;
	fcb->bufCursor = 0;
	fcb->buffer = AllocKMem((size_t) block_size);
	fcb->bufferIsDirty = 0;
	fcb->inodeIsDirty = 0;
	fcb->openCount = 1;
	fcb->index1 = fcb->index2 = fcb->index3 = fcb->index4 = 0;
	fcb->currentBlock = fcb->inode->i_block[0];
	fcb->read = ReadFromFile;
	fcb->write = WriteToFile;

	ReadBlock(fcb->currentBlock, fcb->buffer);
	return fcb;
}

//====================================================
// Create a new file. entry for a file of type type
// Returns an FCB on success or a negative error code
//====================================================
struct FCB *CreateFileWithType(char *name, long type)
{
	char *parentDirectory, *fileName, *dirBuffer;
	u_int32_t parentINodeNo, inodeNo, i;
	u_int16_t recLength, sizeOfEntry, sizeOfNewEntry;
	struct ext2_inode *inode;
	struct ext2_dir_entry_2 *entry, *newEntry;
	struct FCB *dirFcb, *fcb;

	// Does the file already exist?
	if (GetFileINode(name))
		return (struct FCB *) -EEXIST;

	// Get the INode of the parentdirectory
	parentDirectory = AllocUMem((size_t) (strlen(name) + 1));
	strcpy(parentDirectory, name);
	fileName = strrchr(parentDirectory, '/');
	fileName[0] = 0;
	fileName++;
	parentINodeNo = GetFileINode(parentDirectory);
	if (!parentINodeNo)
	{
		DeallocMem(parentDirectory);
		return (struct FCB *) -ENOENT;
	}

	// Create an inode for the new file
	inode = AllocKMem(sizeof(struct ext2_inode));
	memset(inode, 0, sizeof(struct ext2_inode));
	inode->i_mode = (u_int16_t) (EXT2_S_IFREG | EXT2_S_IRUSR | EXT2_S_IWUSR
			| EXT2_S_IRGRP | EXT2_S_IWGRP | EXT2_S_IROTH | EXT2_S_IWOTH);
	inode->i_atime = inode->i_ctime = inode->i_mtime = (u_int32_t) unixtime;
	inode->i_dtime = 0;
	inode->i_links_count = 1;
	inode->i_blocks = 0;
	inodeNo = GetFreeINode(0);

	// Write the new inode to disk
	PutINode(inodeNo, inode);

	// Create a directory entry for the new file
	dirFcb = OpenFileByInodeNumber(parentINodeNo);
	dirBuffer = AllocUMem(dirFcb->inode->i_size);
	(void) ReadFile(dirFcb, dirBuffer, (long) (dirFcb->inode->i_size));
	sizeOfNewEntry = (u_int16_t) (8 + strlen(fileName) + 4
			- strlen(fileName) % 4);
	entry = (struct ext2_dir_entry_2 *) dirBuffer;
	sizeOfEntry = (u_int16_t) (8 + entry->name_len + 4 - entry->name_len % 4);
	while (sizeOfEntry + sizeOfNewEntry > entry->rec_len)
	{
		entry = (struct ext2_dir_entry_2 *) ((char *) entry + entry->rec_len);
		sizeOfEntry =
				(u_int16_t) (8 + entry->name_len + 4 - entry->name_len % 4);
	}

	// There's room for the new entry at the end of this record
	recLength = entry->rec_len;
	entry->rec_len = sizeOfEntry;
	newEntry = AllocUMem(sizeof(struct ext2_dir_entry_2));
	memset(newEntry, 0, sizeof(struct ext2_dir_entry_2));
	newEntry->file_type = (u_int8_t) type;
	newEntry->inode = (u_int32_t) inodeNo;
	newEntry->name_len = (u_int8_t) strlen(fileName);
	newEntry->rec_len = recLength - sizeOfEntry;
	for (i = 0; i < newEntry->name_len; i++)
		newEntry->name[i] = fileName[i];
	memcpy((char *) entry + sizeOfEntry, newEntry, sizeOfNewEntry);
	DeallocMem(parentDirectory);

	// Write the directory buffer back to disk
	(void) Seek(dirFcb, 0, SEEK_SET);
	(void) WriteFile(dirFcb, dirBuffer, (long) (dirFcb->inode->i_size));
	DeallocMem(dirBuffer);
	(void) CloseFile(dirFcb);

	// Create a FCB for the new file
	fcb = AllocKMem(sizeof(struct FCB));
	fcb->inode = inode;
	fcb->inodeNumber = inodeNo;
	fcb->fileCursor = 0;
	fcb->bufCursor = 0;
	fcb->buffer = AllocKMem((size_t) block_size);
	fcb->currentBlock = 0;
	fcb->index1 = fcb->index2 = fcb->index3 = fcb->index4 = 0;
	fcb->read = ReadFromFile;
	fcb->write = WriteToFile;
	return fcb;
}

struct FCB *CreateFile(char *name)
{
	return CreateFileWithType(name, EXT2_FT_REG_FILE);
}

//==============================================
// Open the file "path" and assign an FCB to it
//==============================================
struct FCB *OpenFile(char *path)
{
	struct FCB *fcb = AllocKMem(sizeof(struct FCB));
	fcb->inode = AllocKMem(sizeof(struct ext2_inode));
	fcb->inodeNumber = GetFileINode(path);
	if (!fcb->inodeNumber)
	{
		DeallocMem(fcb);
		return (struct FCB *) -ENOENT;
	}
	GetINode(fcb->inodeNumber, fcb->inode);
	fcb->openCount = 1;
	fcb->fileCursor = 0;
	fcb->bufCursor = 0;
	fcb->buffer = AllocKMem((size_t) block_size);
	fcb->bufferIsDirty = 0;
	fcb->inodeIsDirty = 0;
	fcb->index1 = fcb->index2 = fcb->index3 = fcb->index4 = 0;
	fcb->currentBlock = fcb->inode->i_block[0];
	fcb->read = ReadFromFile;
	fcb->write = WriteToFile;
	ReadBlock(fcb->currentBlock, fcb->buffer);
	return fcb;
}

//===================================
// Close the file represented by fcb
//===================================
long CloseFile(struct FCB *fcb)
{
	fcb->openCount--;
	if (!fcb->openCount)
	{
		if (fcb->bufferIsDirty)
			WriteBlock(fcb->currentBlock, fcb->buffer);
		if (fcb->inodeIsDirty)
			PutINode(fcb->inodeNumber, fcb->inode);
		DeallocMem(fcb->buffer);
		DeallocMem(fcb->inode);
		DeallocMem(fcb);
		FlushCaches();
	}
	return 0;
}

//================================================================
// Read noBytes from the file represented by fcb into buffer
// Return the no of bytes actually read, or a negative error code
//================================================================
long ReadFile(struct FCB *fcb, char *buffer, long noBytes)
{
	int i;

	for (i = 0; i < noBytes; i++)
	{
		if (fcb->fileCursor == (int) (fcb->inode->i_size))
			return i;
		if (fcb->bufCursor == block_size)
		{
			SetBufferFromCursor(fcb);
		}
		buffer[i] = fcb->buffer[fcb->bufCursor];
		fcb->bufCursor++;
		fcb->fileCursor++;
	}
	return noBytes;
}

//================================================================
// Write noBytes from buffer into the file represented by fHandle
// Returns no of bytes written
//================================================================
long WriteFile(struct FCB *fcb, char *buffer, long noBytes)
{
	int i;

	// If noBytes == 0 there is nothing to do
	if (!noBytes)
		return 0;
	// Deal with the case of an initial empty file
	if (!fcb->inode->i_block[0])
		AddFirstBlockToFile(fcb);
	for (i = 0; i < noBytes; i++)
	{
		// If the bufCursor is at the end of the block we need to load the next block
		if (fcb->bufCursor == block_size)
		{
			// If the current buffer is dirty write the block to disk
			if (fcb->bufferIsDirty)
			{
				WriteBlock(fcb->currentBlock, fcb->buffer);
				fcb->bufferIsDirty = 0;
			}
			// If the file cursor is at the end of the file allocate a new block
			if (fcb->fileCursor >= (int) (fcb->inode->i_size - 1))
				AddBlockToFile(fcb);
			else
				// Load the block and set fcb->bufCursor
				SetBufferFromCursor(fcb);
		}
		// Increment the bufCursor and copy the byte to the file buffer
		fcb->buffer[fcb->bufCursor++] = buffer[i];
		// Mark the current buffer as dirty and increment the fileCursor
		fcb->bufferIsDirty = 1;
		fcb->fileCursor++;
		// If at the end of the file increment the file size in the inode and mark the inode as dirty
		if (fcb->fileCursor > (int) (fcb->inode->i_size))
		{
			fcb->inode->i_size++;
			fcb->inodeIsDirty = 1;
		}
	}
	return noBytes;
}

//===============================================
// Delete the file name
// Returns 0 on success or a negative error code
//===============================================
long DeleteFile(char *name)
{
	char *buffer = AllocKMem(block_size);
	char *buffer1 = AllocKMem(block_size);
	__le32 *blocks;
	__le32 *iblocks;
	int i, j;
	char *parentDirectory, *fileName, *dirBuffer;
	u_int32_t parentINodeNo;
	struct FCB *fcb, *dirFcb;
	struct ext2_dir_entry_2 *entry, *prevEntry;

	// Get the INode of the parentdirectory
	parentDirectory = AllocUMem((size_t) strlen(name) + 1);
	strcpy(parentDirectory, name);
	fileName = strrchr(parentDirectory, '/');
	fileName[0] = 0;
	fileName++;
	parentINodeNo = GetFileINode(parentDirectory);
	if (parentINodeNo == (u_int32_t) -ENOENT)
	{
		DeallocMem(buffer);
		DeallocMem(buffer1);
		DeallocMem(parentDirectory);
		return -ENOENT;
	}

	// Now free the blocks allocated to this file
	fcb = OpenFile(name);
	if ((long) fcb < 0)
	{
		DeallocMem(buffer);
		DeallocMem(buffer1);
		DeallocMem(parentDirectory);
		return (long) fcb;
	}

	// Delete all direct blocks
	for (i = 0; i < EXT2_IND_BLOCK; i++)
		if (fcb->inode->i_block[i])
			ClearBlockBitmapBit(fcb->inode->i_block[i]);
	if (fcb->inode->i_block[EXT2_IND_BLOCK])
	{
		// Delete indirect blocks
		ReadBlock(fcb->inode->i_block[EXT2_IND_BLOCK], buffer);
		blocks = (__le32 *) buffer;
		for (i = 0; i < block_size / sizeof(__le32 ); i++)
			if (blocks[i])
				ClearBlockBitmapBit(blocks[i]);
		ClearBlockBitmapBit(fcb->inode->i_block[EXT2_IND_BLOCK]);
	}
	if (fcb->inode->i_block[EXT2_DIND_BLOCK])
	{
		// Delete double-indirect blocks
		ReadBlock(fcb->inode->i_block[EXT2_DIND_BLOCK], buffer);
		blocks = (__le32 *) buffer;
		for (i = 0; i < block_size / sizeof(__le32 ); i++)
		{
			if (blocks[i])
			{
				ReadBlock(blocks[i], buffer1);
				iblocks = (__le32 *) buffer1;
				for (j = 0; j < block_size / sizeof(__le32 ); j++)
					if (iblocks[j])
						ClearBlockBitmapBit(iblocks[j]);
				ClearBlockBitmapBit(blocks[i]);
			}
		}
		ClearBlockBitmapBit(fcb->inode->i_block[EXT2_DIND_BLOCK]);
	}
	if (fcb->inode->i_block[EXT2_TIND_BLOCK])
	{
		// Delete triple-indirect blocks
	}

	// Find the directory entry for the file
	dirFcb = OpenFileByInodeNumber(parentINodeNo);
	dirBuffer = AllocUMem(dirFcb->inode->i_size);
	(void) ReadFile(dirFcb, dirBuffer, (long) (dirFcb->inode->i_size));
	entry = (struct ext2_dir_entry_2 *) dirBuffer;
	prevEntry = entry;
	while (strlen(fileName) != entry->name_len
			|| strncmp(entry->name, fileName, entry->name_len))
	{
		if ((long) entry + entry->name_len - (long) dirBuffer
				> (long) dirFcb->inode->i_size)
		{
			DeallocMem(dirBuffer);
			(void) CloseFile(dirFcb);
			DeallocMem(buffer);
			DeallocMem(buffer1);
			DeallocMem(parentDirectory);
			return -ENOENT;
		}
		prevEntry = entry;
		entry = (struct ext2_dir_entry_2 *) ((char *) entry + entry->rec_len);
	}

	// Entry now points to the directory entry for this file, prevEntry to the previous one
	prevEntry->rec_len += entry->rec_len;
	entry->inode = 0;
	(void) Seek(dirFcb, 0, SEEK_SET);
	(void) WriteFile(dirFcb, dirBuffer, (long) dirFcb->inode->i_size);
	DeallocMem(dirBuffer);
	(void) CloseFile(dirFcb);
	DeallocMem(buffer);
	DeallocMem(buffer1);
	DeallocMem(parentDirectory);

	// Finally, delete the inode
	memset(fcb->inode, 0, sizeof(struct ext2_inode));
	PutINode(fcb->inodeNumber, fcb->inode);
	ClearINodeBitmapBit(fcb->inodeNumber);
	(void) CloseFile(fcb);
	return 0;
}

//===============================================
// Create the directory "path"
// Returns 0 on success or a negative error code
//===============================================
long CreateDir(char *name)
{
	char *parentDirectory, *fileName;
	u_int32_t parentINodeNo, group;
	struct ext2_dir_entry_2 *newEntry;
	struct ext2_inode parentINode;
	struct FCB *fcb = CreateFileWithType(name, EXT2_FT_DIR);

	if (!fcb)
		return -ENOENT;

	// Get inode number of parent directory
	parentDirectory = AllocUMem((size_t) strlen(name) + 1);
	strcpy(parentDirectory, name);
	fileName = strrchr(parentDirectory, '/');
	fileName[0] = 0;
	parentINodeNo = GetFileINode(parentDirectory);
	DeallocMem(parentDirectory);

	newEntry = AllocUMem(sizeof(struct ext2_dir_entry_2));
	memset(newEntry, 0, sizeof(struct ext2_dir_entry_2));
	newEntry->file_type = EXT2_FT_DIR;
	newEntry->inode = fcb->inodeNumber;
	fcb->inode->i_links_count++;
	newEntry->name_len = 1;
	newEntry->rec_len = 12;
	newEntry->name[0] = '.';
	(void) WriteFile(fcb, (char *) newEntry, 12);
	newEntry->inode = parentINodeNo;

	GetINode(parentINodeNo, &parentINode);
	parentINode.i_links_count++;
	PutINode(parentINodeNo, &parentINode);
	newEntry->name_len = 2;
	newEntry->rec_len = 0x3F4;
	newEntry->name[1] = '.';
	(void) WriteFile(fcb, (char *) newEntry, 12);
	group = fcb->inodeNumber / sb.s_inodes_per_group;
	group_descriptors[group].bg_used_dirs_count++;
	fcb->inode->i_mode = (u_int16_t) (EXT2_S_IFDIR | EXT2_S_IRUSR | EXT2_S_IWUSR
			| EXT2_S_IXUSR | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH
			| EXT2_S_IXOTH);
	fcb->inode->i_size = (u_int32_t) block_size;
	(void) CloseFile(fcb);
	return 0;
}

//===========================================================
// Seek to offset from whence in the file represented by fcb
// Returns 0 on success or a negative error code
//===========================================================
long Seek(struct FCB *fcb, int offset, int whence)
{
	u_int32_t blocksNeeded;

	switch (whence)
	{
	case SEEK_SET:
		fcb->fileCursor = offset;
		break;
	case SEEK_CUR:
		fcb->fileCursor += offset;
		break;
	case SEEK_END:
		fcb->fileCursor = (int) (fcb->inode->i_size) + offset;
		break;
	default:
		return -1;
	}

	if (fcb->fileCursor < 0)
		fcb->fileCursor = 0;

	if ((u_int32_t) (fcb->fileCursor) > fcb->inode->i_size)
	{
		// Do we need to add more blocks
		u_int32_t currentBlocks = fcb->inode->i_size / block_size;
		if (fcb->inode->i_size % block_size)
			currentBlocks++;
		blocksNeeded = (u_int32_t) (fcb->fileCursor / block_size + 1)
				- currentBlocks;
		if (fcb->currentBlock == 0 && blocksNeeded > 0)
		{
			// This is a special case where the file is currently empty
			AddFirstBlockToFile(fcb);
			fcb->inode->i_size += block_size;
			blocksNeeded--;
		}

		while (blocksNeeded--)
		{
			AddBlockToFile(fcb);
			fcb->inode->i_size += block_size;
		}
		fcb->inode->i_size = (u_int32_t) (fcb->fileCursor);
	}

	SetBufferFromCursor(fcb);
	return fcb->fileCursor;
}

//================================================
// Truncate the file represented by fcb to length
// Returns 0 on success or a negative error code
//================================================
long Truncate(struct FCB *fcb, u_int32_t length)
{
	if (length == fcb->inode->i_size)
	{
		// Nothing to do
		return 0;
	}
	else if (length < fcb->inode->i_size)
	{
		fcb->inode->i_size = length;
		if (length < (u_int32_t) (fcb->fileCursor))
		{
			fcb->fileCursor = (int) length;
			SetBufferFromCursor(fcb);
		}
	}
	else
		return -EPERM;
	return 0;
}

//=============================
// The actual filesystem task
//=============================
void fsTaskCode(void)
{
	struct Message *FSMsg;
	struct MessagePort *tempPort;
	int result;
	struct FCB *fcb;
	struct FileInfo info;

	FSPort = AllocMessagePort();
	FSMsg = ALLOCMSG;

	kprintf(3, 0, "Starting Filesystem Task");

	FSPort->waitingProc = (struct Task *) -1L;
	FSPort->msgQueue = 0;

	InitializeHD();

	while (1)
	{
		fcb = 0;
		ReceiveMessage(FSPort, FSMsg);
		tempPort = FSMsg->tempPort;
		switch (FSMsg->byte)
		{
		case CREATEFILE:
			fcb = CreateFile((char *) FSMsg->quad1);
			if (fcb > 0)
				fcb->pid = FSMsg->pid;
			FSMsg->quad1 = (long) fcb;
			break;

		case OPENFILE:
			fcb = OpenFile((char *) FSMsg->quad1);
			if ((long) fcb > 0)
				fcb->pid = FSMsg->pid;
			FSMsg->quad1 = (long) fcb;
			break;

		case CLOSEFILE:
			(void) CloseFile((struct FCB *) FSMsg->quad1);
			DeallocMem((void *) FSMsg->quad1);
			break;

		case READFILE:
			FSMsg->quad1 = ReadFile((struct FCB *) FSMsg->quad1,
					(char *) FSMsg->quad2, FSMsg->quad3);
			break;

		case WRITEFILE:
			FSMsg->quad1 = WriteFile((struct FCB *) FSMsg->quad1,
					(char *) FSMsg->quad2, FSMsg->quad3);
			break;

		case DELETEFILE:
			FSMsg->quad1 = DeleteFile((char *) FSMsg->quad1);
			break;

		case TESTFILE:
			if (!strcmp((char *) FSMsg->quad1, "/"))
				result = 1;
			else
			{
				result = (long) GetFileINode((char *) FSMsg->quad1);
				if (result != -ENOENT)
					result = 1;
			}
			FSMsg->quad1 = result;
			break;

		case GETFILEINFO:
			fcb = (struct FCB *) FSMsg->quad1;
			info.inode = (long) fcb->inodeNumber;
			info.mode = (long) fcb->inode->i_mode;
			info.uid = (long) fcb->inode->i_uid;
			info.gid = (long) fcb->inode->i_gid;
			info.size = (long) fcb->inode->i_size;
			info.atime = (long) fcb->inode->i_atime;
			info.ctime = (long) fcb->inode->i_ctime;
			info.mtime = (long) fcb->inode->i_mtime;
			memcpy((char *) FSMsg->quad2, (char *) (&info),
					sizeof(struct FileInfo));
			break;

		case CREATEDIR:
			FSMsg->quad1 = CreateDir((char *) FSMsg->quad1);
			break;

		case SEEK:
			FSMsg->quad1 = Seek((struct FCB *) FSMsg->quad1, FSMsg->quad2,
					FSMsg->quad3);
			break;

		case TRUNCATE:
			FSMsg->quad1 = Truncate((struct FCB*) FSMsg->quad1,
					(u_int32_t) (FSMsg->quad2));
			break;

		default:
			break;
		}
		SendMessage(tempPort, FSMsg);
	}
}
