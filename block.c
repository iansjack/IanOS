// This file contains various functions for dealing with ext2fs blocks

//typedef int umode_t;

//#include <linux/types.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <string.h>
#include <unistd.h>
#include <blocks.h>
#include <memory.h>

#define StartOfGroup(i) firstBlock + i * sb->s_blocks_per_group
#define BASEADDRESS	0xFFFF800000000000L
#define BlockToAdd(b) ((char *)(BASEADDRESS + PartitionStart * 1024 + b * block_size))

void ReadPSector(char *buffer, u_int32_t block_no); // Defined in ide.s
void WritePSector(char *buffer, u_int32_t block_no); // Defined in ide.s

struct ext2_super_block *sb;
int block_size = 1024;
unsigned int no_of_blockgroups;
int inodes_per_block;
struct ext2_group_desc *group_descriptors;
char **block_bitmap;
char **inode_bitmap;
u_int32_t currentDirectory = 2; // The inode # of the currentdirectory
unsigned int PartitionStart;
short inodeMultiplier;
short firstBlock = 0;
//char *(blocks[1024]);

//=============================
// Read a page from disk
//=============================
void ReadPageFromDisk(unsigned long address)
{
	address &= 0xFFFFFFFFFFFFF000;
	unsigned long block = (address &0x7FFFFFFFFFF) / block_size;

	int i;
	for (i = 0; i < 8 /*block_size / SECTOR_SIZE*/; i++)
		ReadPSector((char *)(address + i * SECTOR_SIZE), block * 2 + i); // (block_size / SECTOR_SIZE) + i);
}

//============================
// Write a page to disk
//============================
void WritePageToDisk(unsigned long address)
{
	address &= 0xFFFFFFFFFFFFF000;
	unsigned long block = (address &0x7FFFFFFFFFF) / block_size;

	int i;
	for (i = 0; i < 8 /*block_size / SECTOR_SIZE*/; i++)
		WritePSector((char *)(address + i * SECTOR_SIZE), block * 2 + i); // (block_size / SECTOR_SIZE) + i);
}

//===================================================================
// Read in initial details from the superblock and group descriptors
// Currently this assumes an unpartitioned disk
// We need to read the partition table and find the partition
//===================================================================
void InitializeHD()
{
	struct MBR *mbr = (struct MBR *)BASEADDRESS;
	unsigned int i;

	PartitionStart = 0;

	// Read the MBR and Partition Table
	PartitionStart = ((struct PartTable)(mbr->PT[0])).LBA / 2;	// MBR give offset in sectors, but we work in blocks
	//blocks = (char *[1024])(BASEADDRESS + PartitionStart);

	// Read the SuperBlock
	sb = (struct ext2_super_block *)(BASEADDRESS + (PartitionStart + 1) * 1024);
	block_size = block_size << sb->s_log_block_size;
	inodes_per_block = block_size / sb->s_inode_size;
	inodeMultiplier = sb->s_inode_size / sizeof(struct ext2_inode);
	no_of_blockgroups = sb->s_blocks_count / sb->s_blocks_per_group + 1;

	block_bitmap = AllocUMem(no_of_blockgroups * sizeof(char *)); // block_size);
	inode_bitmap = AllocUMem(no_of_blockgroups * sizeof(char *)); // block_size);

	if (block_size == 1024)
		group_descriptors = (struct ext2_group_desc *)(BlockToAdd(2));
	else
		group_descriptors = (struct ext2_group_desc *)(BlockToAdd(1));

	for (i = 0; i < no_of_blockgroups; i++)
	{
		block_bitmap[i] = BlockToAdd(group_descriptors[i].bg_block_bitmap);
		inode_bitmap[i] = BlockToAdd(group_descriptors[i].bg_inode_bitmap);
	}
//	FlushDisk();
}

//======================================================
// Return the value of the bit for block in the bitmap
//======================================================
int GetBlockBitmapBit(int block)
{
	int group, block_in_group, byte_in_map, bit_in_byte;

	block--;
	group = block / sb->s_blocks_per_group;
	block_in_group = block % sb->s_blocks_per_group;
	byte_in_map = block_in_group / 8;
	bit_in_byte = block_in_group % 8;
	return ((1 << bit_in_byte)
			& *(*block_bitmap + block_size * group + byte_in_map));
}

//======================================================
// Set the value of the bit for block in the bitmap
//======================================================
void SetBlockBitmapBit(u_int32_t block)
{
	u_int32_t group, block_in_group, byte_in_map, bit_in_byte;

	block--;
	group = block / sb->s_blocks_per_group;
	block_in_group = block % sb->s_blocks_per_group;
	byte_in_map = block_in_group / 8;
	bit_in_byte = block_in_group % 8;
	*(*block_bitmap + block_size * group + byte_in_map) |= (1 << bit_in_byte);
	group_descriptors[group].bg_free_blocks_count--;
	sb->s_free_blocks_count--;
}

//======================================================
// Clear the value of the bit for block in the bitmap
//======================================================
void ClearBlockBitmapBit(u_int32_t block)
{
	u_int32_t group, block_in_group, byte_in_map, bit_in_byte;

	block--;
	group = block / sb->s_blocks_per_group;
	block_in_group = block % sb->s_blocks_per_group;
	byte_in_map = block_in_group / 8;
	bit_in_byte = block_in_group % 8;
	*(*block_bitmap + block_size * group + byte_in_map) &= ~(1 << bit_in_byte);
	group_descriptors[group].bg_free_blocks_count++;
	sb->s_free_blocks_count++;
}

//=====================================================================
// Return the (absolute) block number of the first free block in group
// Mark the block as used
//=====================================================================
u_int32_t GetFreeBlock(u_int32_t group)
{
	u_int32_t block;
	while (group_descriptors[group].bg_free_blocks_count == 0) group++;
	int i = 0, j=0;
	while ((*block_bitmap)[block_size * group + i] == -1)
		i++;
	for (j = 0; j < 8; j++)
		if (!((*block_bitmap)[block_size * group + i] & (1 << j)))
			break;
	block = group * sb->s_blocks_per_group + 8 * i + j +1;

	// Mark this block as in use
	SetBlockBitmapBit(block);
	return block;
}

//======================================================
// Return the value of the bit for inode in the bitmap
//======================================================
int GetINodeBitmapBit(int inode)
{
	int group, inode_in_group, byte_in_map, bit_in_byte;

	inode--;
	group = inode / sb->s_inodes_per_group;
	inode_in_group = inode % sb->s_inodes_per_group;
	byte_in_map = inode_in_group / 8;
	bit_in_byte = inode_in_group % 8;
	return ((1 << bit_in_byte)
			& *(*inode_bitmap + block_size * group + byte_in_map));
}

//======================================================
// Set the value of the bit for inode in the bitmap
//======================================================
void SetINodeBitmapBit(u_int32_t inode)
{
	u_int32_t group, inode_in_group, byte_in_map, bit_in_byte;

	inode--;
	group = inode / sb->s_inodes_per_group;
	inode_in_group = inode % sb->s_inodes_per_group;
	byte_in_map = inode_in_group / 8;
	bit_in_byte = inode_in_group % 8;
	*(*inode_bitmap + block_size * group + byte_in_map) |= (1 << bit_in_byte);
	group_descriptors[group].bg_free_inodes_count--;
	sb->s_free_inodes_count--;
}

//======================================================
// Clear the value of the bit for inode in the bitmap
//======================================================
void ClearINodeBitmapBit(u_int32_t inode)
{
	u_int32_t group, inode_in_group, byte_in_map, bit_in_byte;

	inode--;
	group = inode / sb->s_inodes_per_group;
	inode_in_group = inode % sb->s_inodes_per_group;
	byte_in_map = inode_in_group / 8;
	bit_in_byte = inode_in_group % 8;
	*(*inode_bitmap + block_size * group + byte_in_map) &= ~(1 << bit_in_byte);
	group_descriptors[group].bg_free_inodes_count++;
	sb->s_free_inodes_count++;
}

//=====================================================================
// Return the (absolute) inode number of the first free inode in group
//=====================================================================
u_int32_t GetFreeINode(u_int32_t group)
{
	u_int32_t inode;
	int i = 0, j = 0;
	while ((*inode_bitmap)[block_size * group + i] == -1)
		i++;
	for (j = 0; j < 8; j++)
		if (!((*inode_bitmap)[block_size * group + i] & (1 << j)))
			break;
	inode = group * sb->s_inodes_per_group + 8 * i + j + 1;

	// Mark this inode as in use
	SetINodeBitmapBit(inode);
	return inode;
}

/*
//=====================================================================
// Flushes the cached values of the superblock, group descriptors, and
// block and inode bitmaps back to disk
//=====================================================================
void FlushCaches(void)
{
	// We need to test whether we are dealing with SPARSE_SUPERBLOCKS or not. Assume yes for the time being
	u_int32_t i;
	char *buffer = AllocUMem(block_size);
	memset(buffer, 0, (size_t)block_size);

	for (i = 0; i < no_of_blockgroups; i++)
		memcpy(buffer + i * sizeof(struct ext2_group_desc),
				(void *) &group_descriptors[i], sizeof(struct ext2_group_desc));

	for (i = 0; i < no_of_blockgroups; i++)
	{
		// Write SuperBlock and Group Descriptors. Assume < 25 block_groups
		if (i == 0 || i == 1 || i == 3 || i == 5 || i == 7 || i == 9)
		{
			sb->s_block_group_nr = (u_int16_t)i;
			if (i == 0)
			{
				// WritePSector((char *)&sb, PartitionStart * (block_size / SECTOR_SIZE) + 2);
				// WritePSector((char *)&sb + SECTOR_SIZE, PartitionStart * (block_size / SECTOR_SIZE) + 3);
			}
			else
				WriteBlock(StartOfGroup(i), (char *) &sb);
			WriteBlock(StartOfGroup(i) + 1, buffer);
		}
		WriteBlock(group_descriptors[i].bg_block_bitmap,
				&block_bitmap[i * block_size]);
		WriteBlock(group_descriptors[i].bg_inode_bitmap,
				&inode_bitmap[i * block_size]);
	}
	DeallocMem(buffer);
}
*/

//===================================================================
// Fill in the struct inode with the details from inode inodeNumber
//===================================================================
void GetINode(u_int32_t inodeNumber, struct ext2_inode *inode)
{
	u_int32_t group_number, block_in_group, block, number_in_block;
	char *buffer;
	struct ext2_inode *inodes;

	inodeNumber--;		// inodes are numbered from 1, arrays from 0
	group_number = inodeNumber / sb->s_inodes_per_group;
	block_in_group = (inodeNumber % sb->s_inodes_per_group)
			/ inodes_per_block;
	block = group_descriptors[group_number].bg_inode_table + block_in_group;
	number_in_block = (inodeNumber % sb->s_inodes_per_group)
			% inodes_per_block;
	buffer = BlockToAdd(block);
	inodes = (struct ext2_inode *) buffer;
	memcpy((void *) inode, (void *) &inodes[number_in_block * inodeMultiplier],
			sizeof(struct ext2_inode));
}

//===================================================================
// Write the struct inode
//===================================================================
void PutINode(u_int32_t inodeNumber, struct ext2_inode *inode)
{
	u_int32_t group_number, block_in_group, block, number_in_block;
	char *buffer;
	struct ext2_inode *inodes;

	inodeNumber--;
	group_number = inodeNumber / sb->s_inodes_per_group;
	block_in_group = (inodeNumber % sb->s_inodes_per_group)
			/ inodes_per_block;
	block = group_descriptors[group_number].bg_inode_table + block_in_group;
	number_in_block = (inodeNumber % sb->s_inodes_per_group)
			% inodes_per_block;
	buffer = BlockToAdd(block);
	inodes = (struct ext2_inode *) buffer;
	memcpy((void *) &inodes[number_in_block * inodeMultiplier], (void *) inode,
			sizeof(struct ext2_inode));
}

//===========================================================
// Returns the inode number of path or a negative error code
//===========================================================
__le32 GetFileINode(char *path)
{
	char *temp, *name;
	char *buffer;
	u_int32_t startingDirectory = currentDirectory;

	temp = AllocUMem(strlen(path) + 1);
	strcpy(temp, path);
	if (path[0] == '/')
	{
		path++;
		startingDirectory = 2; // Root directory
	}
	name = strtok(temp, "/");

	while (name)
	{
		struct ext2_inode parentINode;
		struct ext2_dir_entry_2 *dir;
		int dir_pos;

		GetINode(startingDirectory, &parentINode);
		buffer = BlockToAdd(parentINode.i_block[0]);
		dir = (struct ext2_dir_entry_2 *) buffer;
		dir_pos = 0;
		while (1)
		{
			dir = (struct ext2_dir_entry_2 *) (buffer + dir_pos);
			if (!strncmp(dir->name, name, dir->name_len)
					&& strlen(name) == dir->name_len)
				break;
			dir_pos += dir->rec_len;
			if (dir_pos >= (int)(parentINode.i_size))
			{
				DeallocMem(temp);
				// DeallocMem(buffer);
				return 0;
			}
		}
		startingDirectory = dir->inode;
		name = strtok(/*NULL*/ 0, "/");
	}
	DeallocMem(temp);
	return startingDirectory;
}

//======================================================================
// Adjust the fcb->index? fields to reflect the current fcb->fileCursor
//======================================================================
void SetIndexesFromCursor(struct FCB *fcb)
{
	fcb->bufCursor = fcb->fileCursor % block_size;
	fcb->index1 = fcb->fileCursor / block_size;
	if (fcb->index1 < DIRECT_BLOCKS)
	{
		fcb->index2 = fcb->index3 = fcb->index4 = 0;
	}
	else if (fcb->index1 < (int)(DIRECT_BLOCKS + INDIRECT_BLOCKS))
	{
		fcb->index2 = fcb->index1 - DIRECT_BLOCKS;
		fcb->index3 = fcb->index4 = 0;
		fcb->index1 = EXT2_IND_BLOCK;
	}
	else if (fcb->index1
			< (int)(DIRECT_BLOCKS + INDIRECT_BLOCKS + DOUBLE_INDIRECT_BLOCKS))
	{
		fcb->index2 = (int)((fcb->index1 - DIRECT_BLOCKS - INDIRECT_BLOCKS)
				/ INDIRECT_BLOCKS);
		fcb->index3 = (int)((fcb->index1 - DIRECT_BLOCKS - INDIRECT_BLOCKS)
				% INDIRECT_BLOCKS);
		fcb->index4 = 0;
		fcb->index1 = EXT2_DIND_BLOCK;
	}
	else
	{
		fcb->index3 = (int)((fcb->index1 - DIRECT_BLOCKS - INDIRECT_BLOCKS
				- DOUBLE_INDIRECT_BLOCKS) / INDIRECT_BLOCKS);
		fcb->index4 = (int)((fcb->index1 - DIRECT_BLOCKS - INDIRECT_BLOCKS
				- DOUBLE_INDIRECT_BLOCKS) % INDIRECT_BLOCKS);
		fcb->index2 = (int)(fcb->index3 / INDIRECT_BLOCKS);
		fcb->index3 = fcb->index3 % INDIRECT_BLOCKS;
		fcb->index1 = EXT2_TIND_BLOCK;
	}
}

//===========================================================
// Update fcb->buffer to reflect the current fcb->fileCursor
//===========================================================
void SetBufferFromCursor(struct FCB *fcb)
{
	SetIndexesFromCursor(fcb);
	fcb->currentBlock = fcb->inode->i_block[fcb->index1];
	fcb->buffer = BlockToAdd(fcb->currentBlock);			// Is this correct???
	if (fcb->index1 >= EXT2_IND_BLOCK)
	{
		__le32 *blocks = (__le32 *) fcb->buffer;
		fcb->currentBlock = blocks[fcb->index2];
		fcb->buffer = BlockToAdd(fcb->currentBlock);
	}
	if (fcb->index1 >= EXT2_DIND_BLOCK)
	{
		__le32 *blocks = (__le32 *) fcb->buffer;
		fcb->currentBlock = blocks[fcb->index3];
		fcb->buffer = BlockToAdd(fcb->currentBlock);
	}
	if (fcb->index1 == EXT2_TIND_BLOCK)
	{
		__le32 *blocks = (__le32 *) fcb->buffer;
		fcb->currentBlock = blocks[fcb->index4];
		fcb->buffer = BlockToAdd(fcb->currentBlock);
	}
	fcb->bufCursor = fcb->fileCursor % block_size;
}

//====================================================
// Add a block to a file that has no allocated blocks
//====================================================
void AddFirstBlockToFile(struct FCB *fcb)
{
	fcb->currentBlock = GetFreeBlock(fcb->inodeNumber / sb->s_inodes_per_group);
	fcb->buffer = BlockToAdd(fcb->currentBlock);
	fcb->inode->i_block[0] = fcb->currentBlock;
	fcb->inode->i_blocks = (u_int32_t)(block_size / INODE_BLOCK_SIZE);
}

//=======================================================================
// Add a new block the the file represented by fcb
// This is quite complicated because of the various level of indirection
//=======================================================================
void AddBlockToFile(struct FCB *fcb)
{
	char *buffer; //  = AllocUMem(1024);
	u_int32_t group = fcb->inodeNumber / sb->s_inodes_per_group;
	__le32 tempBlock, tempBlock2, tempBlock3;
	__le32 *blocks;
	int temp = fcb->fileCursor;

	// If we are not at the end of the file we need to go there (but save the current position!)
	if (temp != (int)(fcb->inode->i_size) - 1)
	{
			fcb->fileCursor = (int)(fcb->inode->i_size) - 1;
			//SetBufferFromCursor(fcb);
	}
	SetBufferFromCursor(fcb);
	memset(buffer, 0, 1024);
	fcb->currentBlock = GetFreeBlock(group);
	if (fcb->index1 < EXT2_IND_BLOCK)
	{
		fcb->index1++;
		if (fcb->index1 < EXT2_IND_BLOCK)
			// This is a direct block
			fcb->inode->i_block[fcb->index1] = fcb->currentBlock;
		else	// fcb->index1 == EXT2_IND_BLOCK
		{
			// This is the first indirect block. Add a block for the indirect block table
			fcb->index2 = 0;
			tempBlock = fcb->inode->i_block[fcb->index1] = (u_int32_t)GetFreeBlock(
					group);
			// Don't forget to increment the block count to count this block
			fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
			// Add currentBlock as the first entry in the indirect block table
			blocks = (__le32 *) buffer;
			blocks[0] = fcb->currentBlock;
		}
	}
	else if (fcb->index1 == EXT2_IND_BLOCK)
	{
		fcb->index2++;
		if (fcb->index2 < (int)(block_size / sizeof(__le32)))
		{
			// This is an indirect block, so get the indirect block table
			// ReadBlock(fcb->inode->i_block[fcb->index1], buffer);
			buffer = BlockToAdd(fcb->inode->i_block[fcb->index1]);
			blocks = (__le32 *) buffer;
			// Add currentBlock to the indirect block table
			blocks[fcb->index2] = fcb->currentBlock;
		}
		else
		{
			// This is the first double-indirect block. Add a block for the first-level indirect block table
			fcb->index1 = EXT2_DIND_BLOCK;
			fcb->index2 = 0;
			fcb->index3 = 0;
			tempBlock = fcb->inode->i_block[fcb->index1] = (u_int32_t)GetFreeBlock(
					group);
			// Don't forget to increment the block count to count this block
			fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
			// Add a block for the second-level indirect block table
			blocks = (__le32 *) buffer;
			tempBlock2 = blocks[0] = GetFreeBlock(group);
			// Don't forget to increment the block count to count this block
			fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
			// Add current block to the indirect-indirect block table
			blocks[0] = fcb->currentBlock;
		}
	}
	else if (fcb->index1 == EXT2_DIND_BLOCK)
	{
		fcb->index3++;
		if (fcb->index3 < (int)(block_size / sizeof(__le32)))
		{
			// This is a double-indirect block, so get the first-level indirect block table
			buffer = BlockToAdd(fcb->inode->i_block[fcb->index1]);
			// Get the second-level indirect block table for index2
			blocks = (__le32 *) buffer;
			tempBlock = blocks[fcb->index2];
			buffer = BlockToAdd(tempBlock);
			//Add currentBlock to the double-indirect block table
			blocks[fcb->index3] = fcb->currentBlock;
		}
		else
		{
			fcb->index3 = 0;
			fcb->index2++;
			if (fcb->index2 < (int)(block_size / sizeof(__le32)))
			{
				// Still a double-indirect block, but we need to add another first-level table
				buffer = BlockToAdd(fcb->inode->i_block[fcb->index1]);
				blocks = (__le32 *) buffer;
				tempBlock = blocks[fcb->index2] = GetFreeBlock(group);
				// Don't forget to increment the block count to count this block
				fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
				// Clear buffer
				memset(buffer, 0, (size_t)block_size);
				blocks[0] = fcb->currentBlock;
			}
			else
			{
				// This is the first triple-indirect block. Add a block for the first-level indirect block table
				// and one for the second-level indirect table
				fcb->index1 = EXT2_TIND_BLOCK;
				fcb->index2 = 0;
				fcb->index3 = 0;
				fcb->index4 = 0;
				tempBlock = fcb->inode->i_block[fcb->index1] =
						(u_int32_t)GetFreeBlock(group);
				// Don't forget to increment the block count to count this block
				fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
				// Add a block for the second-level indirect block table
				blocks = (__le32 *) buffer;
				tempBlock2 = blocks[0] = GetFreeBlock(group);
				// Don't forget to increment the block count to count this block
				fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
				// Add a block for the third-level indirect block table
				tempBlock3 = blocks[0] = GetFreeBlock(group);
				// Don't forget to increment the block count to count this block
				fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
				// Add current block to the indirect-indirect-indirect block table
				blocks[0] = fcb->currentBlock;
			}
		}
	}
	else if (fcb->index1 == EXT2_TIND_BLOCK)
	{
		// *** TO DO ***
		fcb->index4++;
	}
	fcb->inode->i_blocks += I_BLOCK_SIZE_RATIO;
	fcb->inodeIsDirty = 1;

	// Recover the current file position;
	if (temp != fcb->fileCursor)
	{
		fcb->fileCursor = temp;
	}
	SetBufferFromCursor(fcb);
	// DeallocMem(buffer);
}
