#include <kernel.h>

//typedef int umode_t;

#include <linux/types.h>
#include <sys/errno.h>
#include "ext2_fs.h"
#include "blocks.h"

char *strrchr(char *string, char delimiter);

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

//==============================================
// Open the file "path" and assign an FCB to it
//==============================================
struct FCB *OpenFileByInodeNumber(int inode)
{
	struct FCB *fcb = AllocKMem((long)sizeof(struct FCB));
	fcb->inode = AllocKMem((long)sizeof(struct ext2_inode));
	fcb->inodeNumber = inode;
	GetINode(inode, fcb->inode);
	fcb->nextFCB = 0;
	fcb->fileCursor = 0;
	fcb->bufCursor = 0;
	fcb->buffer = AllocKMem(block_size);
	fcb->bufferIsDirty = 0;
	fcb->inodeIsDirty = 0;
	fcb->index1 = fcb->index2 = fcb->index3 = fcb->index4 = 0;
	fcb->currentBlock = fcb->inode->i_block[0];
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
	int parentINodeNo, inodeNo, sizeOfEntry, sizeOfNewEntry, recLength, i;
	struct ext2_inode *inode;
	struct ext2_dir_entry_2 *entry, *newEntry;
	struct FCB *dirFcb;

	// Does the file already exist?
	if (GetFileINode(name))
		return (struct FCB *) -EEXIST;

	// Get the INode of the parentdirectory
	parentDirectory = AllocUMem(strlen(name) + 1);
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
	inode = AllocKMem((int)sizeof(struct ext2_inode));
	memset(inode, 0, sizeof(struct ext2_inode));
	inode->i_mode = (u_int16_t)(EXT2_S_IFREG | EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IRGRP
			| EXT2_S_IWGRP | EXT2_S_IROTH | EXT2_S_IWOTH);
	inode->i_atime = inode->i_ctime = inode->i_mtime = (u_int32_t)unixtime;
	inode->i_dtime = 0;
	inode->i_links_count = 1;
	inode->i_blocks = 0;
	inodeNo = GetFreeINode(0);

	// Write the new inode to disk
	PutINode(inodeNo, inode);

	// Create a directory entry for the new file
	dirFcb = OpenFileByInodeNumber(parentINodeNo);
	dirBuffer = AllocUMem((int)(dirFcb->inode->i_size));
	(void)ReadFile(dirFcb, dirBuffer, (long)(dirFcb->inode->i_size));
	sizeOfNewEntry = 8 + strlen(fileName) + 4 - strlen(fileName) % 4;
	entry = (struct ext2_dir_entry_2 *) dirBuffer;
	sizeOfEntry = 8 + entry->name_len + 4 - entry->name_len % 4;
	while (sizeOfEntry + sizeOfNewEntry > entry->rec_len)
	{
		entry = (struct ext2_dir_entry_2 *) ((char *) entry + entry->rec_len);
		sizeOfEntry = 8 + entry->name_len + 4 - entry->name_len % 4;
	}

	// There's room for the new entry at the end of this record
	recLength = entry->rec_len;
	entry->rec_len = sizeOfEntry;
	newEntry = AllocUMem((long)sizeof(struct ext2_dir_entry_2));
	memset(newEntry, 0, sizeof(struct ext2_dir_entry_2));
	newEntry->file_type = type;
	newEntry->inode = inodeNo;
	newEntry->name_len = strlen(fileName);
	newEntry->rec_len = recLength - sizeOfEntry;
	for (i = 0; i < newEntry->name_len; i++)
		newEntry->name[i] = fileName[i];
	memcpy((char *) entry + sizeOfEntry, newEntry, sizeOfNewEntry);
	DeallocMem(parentDirectory);

	// Write the directory buffer back to disk
	Seek(dirFcb, 0, SEEK_SET);
	WriteFile(dirFcb, dirBuffer, dirFcb->inode->i_size);
	DeallocMem(dirBuffer);
	CloseFile(dirFcb);

	// Create a FCB for the new file
	struct FCB *fcb = AllocKMem(sizeof(struct FCB));
	fcb->inode = inode;
	fcb->inodeNumber = inodeNo;
	fcb->nextFCB = 0;
	fcb->fileCursor = 0;
	fcb->bufCursor = 0;
	fcb->buffer = AllocKMem(block_size);
	fcb->currentBlock = 0;
	fcb->index1 = fcb->index2 = fcb->index3 = fcb->index4 = 0;
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
	fcb->nextFCB = 0;
	fcb->fileCursor = 0;
	fcb->bufCursor = 0;
	fcb->buffer = AllocKMem(block_size);
	fcb->bufferIsDirty = 0;
	fcb->inodeIsDirty = 0;
	fcb->index1 = fcb->index2 = fcb->index3 = fcb->index4 = 0;
	fcb->currentBlock = fcb->inode->i_block[0];
	ReadBlock(fcb->currentBlock, fcb->buffer);
	return fcb;
}

//===================================
// Close the file represented by fcb
//===================================
long CloseFile(struct FCB *fcb)
{
	if (fcb->bufferIsDirty)
		WriteBlock(fcb->currentBlock, fcb->buffer);
	if (fcb->inodeIsDirty)
		PutINode(fcb->inodeNumber, fcb->inode);
	DeallocMem(fcb->buffer);
	DeallocMem(fcb->inode);
	DeallocMem(fcb);
	FlushCaches();
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
		if (fcb->fileCursor == fcb->inode->i_size)
			return i;
		if (fcb->bufCursor == block_size)
		{
			SetBufferFromCursor(fcb);
			fcb->bufCursor = 0;
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
	// If noBytes == 0 there is nothing to do
	if (!noBytes)
		return 0;
	// Deal with the case of an initial empty file
	if (!fcb->inode->i_block[0])
		AddFirstBlockToFile(fcb);
	int i;
	for (i = 0; i < noBytes; i++)
	{
		if (fcb->fileCursor == fcb->inode->i_size)
		{
			fcb->inode->i_size++;
			fcb->inodeIsDirty = 1;
		}
		if (fcb->bufCursor == block_size)
		{
			if (fcb->bufferIsDirty)
				WriteBlock(fcb->currentBlock, fcb->buffer);
			if (fcb->fileCursor < fcb->inode->i_size - 1)
				SetBufferFromCursor(fcb);
			else
				// We need to allocate a new block to the file
				AddBlockToFile(fcb);
			fcb->bufCursor = 0;
		}
		fcb->buffer[fcb->bufCursor++] = buffer[i];
		fcb->bufferIsDirty = 1;
		fcb->fileCursor++;
	}
	return noBytes;
}

//===============================================
// Delete the file name
// Returns 0 on success or a negative error code
//===============================================
long DeleteFile(char *name)
{
	char buffer[block_size];
	char buffer1[block_size];
	char buffer2[block_size];
	__le32 *blocks;
	__le32 *iblocks;
	__le32 *dblocks;
	int i, j, k;

	// Get the INode of the parentdirectory
	char *parentDirectory = AllocUMem(strlen(name) + 1);
	strcpy(parentDirectory, name);
	char *fileName = strrchr(parentDirectory, '/');
	fileName[0] = 0;
	fileName++;
	int parentINodeNo = GetFileINode(parentDirectory);
	if (parentINodeNo == -ENOENT)
	{
		DeallocMem(parentDirectory);
		return -ENOENT;
	}

	// Now free the blocks allocated to this file
	struct FCB *fcb = OpenFile(name);
	if ((long)fcb < 0)
		return (long) fcb;
	// Delete all direct blocks
	for (i = 0; i < EXT2_IND_BLOCK; i++)
		if (fcb->inode->i_block[i])
			ClearBlockBitmapBit(fcb->inode->i_block[i]);
	if (fcb->inode->i_block[EXT2_IND_BLOCK])
	{
		// Delete indirect blocks
		ReadBlock(fcb->inode->i_block[EXT2_IND_BLOCK], buffer);
		blocks = (__le32 *) buffer;
		for (i = 0; i < block_size / 32; i++)
			if (blocks[i])
				ClearBlockBitmapBit(blocks[i]);
		ClearBlockBitmapBit(fcb->inode->i_block[EXT2_IND_BLOCK]);
	}
	if (fcb->inode->i_block[EXT2_DIND_BLOCK])
	{
		// Delete double-indirect blocks
		ReadBlock(fcb->inode->i_block[EXT2_DIND_BLOCK], buffer);
		blocks = (__le32 *) buffer;
		for (i = 0; i < block_size / 32; i++)
		{
			if (blocks[i])
			{
				ReadBlock(blocks[i], buffer1);
				iblocks = (__le32 *) buffer1;
				for (j = 0; j < block_size / 32; j++)
					if (iblocks[j])
						ClearBlockBitmapBit(iblocks[j]);
				ClearBlockBitmapBit(blocks[i]);
			}
			ClearBlockBitmapBit(fcb->inode->i_block[EXT2_DIND_BLOCK]);
		}
	}
	if (fcb->inode->i_block[EXT2_TIND_BLOCK])
	{
		// Delete triple-indirect blocks
	}

	// Find the directory entry for thefile
	struct FCB *dirFcb = OpenFileByInodeNumber(parentINodeNo);
	char *dirBuffer = AllocUMem(dirFcb->inode->i_size);
	ReadFile(dirFcb, dirBuffer, dirFcb->inode->i_size);
	struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *) dirBuffer;
	struct ext2_dir_entry_2 *prevEntry = entry;
	while (strlen(fileName) != entry->name_len
			|| strncmp(entry->name, fileName, entry->name_len))
	{
		if ((long) entry + entry->name_len - (long) buffer
				> dirFcb->inode->i_size)
		{
			DeallocMem(dirBuffer);
			CloseFile(dirFcb);
			DeallocMem(parentDirectory);
			return -ENOENT;
		}
		prevEntry = entry;
		entry = (struct ext2_dir_entry_2 *) ((char *) entry + entry->rec_len);
	}
	// Entry now points to the directory entry for this file, prevEntry to the previous one
	prevEntry->rec_len += entry->rec_len;
	Seek(dirFcb, 0, SEEK_SET);
	WriteFile(dirFcb, dirBuffer, dirFcb->inode->i_size);
	DeallocMem(dirBuffer);
	CloseFile(dirFcb);
	DeallocMem(parentDirectory);

	// Finally, delete the inode
	memset(fcb->inode, 0, sizeof(struct ext2_inode));
	PutINode(fcb->inodeNumber, fcb->inode);
	ClearINodeBitmapBit(fcb->inodeNumber);
	CloseFile(fcb);
	return 0;
}

//===============================================
// Create the directory "path"
// Returns 0 on success or a negative error code
//===============================================
long CreateDir(char *name)
{
	struct FCB *fcb = CreateFileWithType(name, EXT2_FT_DIR);
	if (!fcb)
		return -ENOENT;

	// Get inode number of parent directory
	char *parentDirectory = AllocUMem(strlen(name) + 1);
	strcpy(parentDirectory, name);
	char *fileName = strrchr(parentDirectory, '/');
	fileName[0] = 0;
	int parentINodeNo = GetFileINode(parentDirectory);
	DeallocMem(parentDirectory);

	struct ext2_dir_entry_2 *newEntry = AllocUMem(
			sizeof(struct ext2_dir_entry_2));
	memset(newEntry, 0, sizeof(struct ext2_dir_entry_2));
	newEntry->file_type = EXT2_FT_DIR;
	newEntry->inode = fcb->inodeNumber;
	fcb->inode->i_links_count++;
	newEntry->name_len = 1;
	newEntry->rec_len = 12;
	newEntry->name[0] = '.';
	WriteFile(fcb, (char *) newEntry, 12);
	newEntry->inode = parentINodeNo;
	struct ext2_inode parentINode;
	GetINode(parentINodeNo, &parentINode);
	parentINode.i_links_count++;
	PutINode(parentINodeNo, &parentINode);
	newEntry->name_len = 2;
	newEntry->rec_len = 0x3F4;
	newEntry->name[1] = '.';
	WriteFile(fcb, (char *) newEntry, 12);
	int group = fcb->inodeNumber / sb.s_inodes_per_group;
	group_descriptors[group].bg_used_dirs_count++;
	fcb->inode->i_mode = EXT2_S_IFDIR | EXT2_S_IRUSR | EXT2_S_IWUSR
			| EXT2_S_IXUSR | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH
			| EXT2_S_IXOTH;
	fcb->inode->i_size = block_size;
	CloseFile(fcb);
	return 0;
}

//===========================================================
// Seek to offset from whence in the file represented by fcb
// Returns 0 on success or a negative error code
//===========================================================
long Seek(struct FCB *fcb, int offset, int whence)
{
	switch (whence)
	{
	case SEEK_SET:
		fcb->fileCursor = offset;
		break;
	case SEEK_CUR:
		fcb->fileCursor += offset;
		break;
	case SEEK_END:
		fcb->fileCursor = fcb->inode->i_size + offset;
		break;
	default:
		return -1;
	}

	if (fcb->fileCursor < 0)
		fcb->fileCursor = 0;

	if (fcb->fileCursor > fcb->inode->i_size)
	{
		// Do we need to add more blocks
		int blocksNeeded = fcb->fileCursor / block_size + 1;
		while (blocksNeeded > fcb->inode->i_blocks)
			AddBlockToFile(fcb);
		fcb->inode->i_size = fcb->fileCursor;
	}

	SetBufferFromCursor(fcb);
	fcb->bufCursor = fcb->fileCursor % block_size;
	return fcb->fileCursor;
}

//================================================
// Truncate the file represented by fcb to length
// Returns 0 on success or a negative error code
//================================================
long Truncate(struct FCB *fcb, long length)
{
	if (length == fcb->inode->i_size)
	{
		// Nothing to do
		return 0;
	}
	else if (length < fcb->inode->i_size)
	{
		fcb->inode->i_size = length;
		if (length < fcb->fileCursor)
		{
			fcb->fileCursor = length;
			SetBufferFromCursor(fcb);
			fcb->bufCursor = fcb->fileCursor % block_size;
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
	kprintf(3, 0, "Starting Filesystem Task");

	FSPort = AllocMessagePort();

	struct Message *FSMsg;
	struct MessagePort *tempPort;

	FSMsg = (struct Message *) ALLOCMSG;

	int result;
	struct FCB *fcb;

	FSPort->waitingProc = (struct Task *) -1L;
	FSPort->msgQueue = 0;

	InitializeHD();

	while (1)
	{
		struct FCB *fcb = 0;
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
			fcb = OpenFile((unsigned char *) FSMsg->quad1);
			if ((long) fcb > 0)
				fcb->pid = FSMsg->pid;
			FSMsg->quad1 = (long) fcb;
			break;

		case CLOSEFILE:
			CloseFile((struct FCB *) FSMsg->quad1);
			DeallocMem((void *) FSMsg->quad1);
			break;

		case READFILE:
			FSMsg->quad1 = ReadFile((struct FCB *) FSMsg->quad1, (char *) FSMsg->quad2,
					FSMsg->quad3);
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
			struct FileInfo info;
			info.inode = fcb->inodeNumber;
			info.mode = fcb->inode->i_mode;
			info.uid = fcb->inode->i_uid;
			info.gid = fcb->inode->i_gid;
			info.size = fcb->inode->i_size;
			info.atime = fcb->inode->i_atime;
			info.ctime = fcb->inode->i_ctime;
			info.mtime = fcb->inode->i_mtime;
			copyMem((char *) (&info), (char *) FSMsg->quad2,
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
			FSMsg->quad1 = Truncate((struct FCB*) FSMsg->quad1, FSMsg->quad2);
			break;

		default:
			break;
		}
		SendMessage(tempPort, FSMsg);
	}
}
