// This file contains various functions for dealing with ext2fs blocks

typedef int umode_t;

#include <linux/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ext2_fs.h>
#include <blocks.h>
#include <memory.h>

void ReadPSector(char *buffer, long block_no); // Defined in ide.s
void WritePSector(char *buffer, long block_no); // Defined in ide.s

struct ext2_super_block sb;
int block_size = 1024;
unsigned int no_of_blockgroups;
int inodes_per_block;
struct ext2_group_desc *group_descriptors;
char *block_bitmap;
char *inode_bitmap;
int currentDirectory = 2; // The inode # of the currentdirectory

#define SECTOR_SIZE	512

//=============================
// Read a (fs) block from disk
//=============================
void ReadBlock(int block, char *buffer)
{
	int i;
	for (i = 0; i < block_size / SECTOR_SIZE; i++)
		ReadPSector(buffer + i * SECTOR_SIZE, block * (block_size / SECTOR_SIZE) + i);
}

//============================
// Write a (fs) block to disk
//============================
void WriteBlock(int block, char *buffer)
{
	int i;
	for (i = 0; i < block_size / SECTOR_SIZE; i++)
		WritePSector(buffer + i * SECTOR_SIZE, block * (block_size / SECTOR_SIZE) + i);
}

//===================================================================
// Read in initial details from the superblock and group descriptors
//===================================================================
void InitializeHD()
{
	char buffer[block_size];
	unsigned int i;

	ReadBlock(1, (char *) &sb);
	block_size = block_size << sb.s_log_block_size;
	inodes_per_block = block_size / (int)sizeof(struct ext2_inode);
	no_of_blockgroups = sb.s_blocks_count / sb.s_blocks_per_group + 1;
	group_descriptors = AllocUMem(
			no_of_blockgroups * (long)sizeof(struct ext2_group_desc));
	block_bitmap = AllocUMem((long)(no_of_blockgroups * block_size));
	inode_bitmap = AllocUMem((long)(no_of_blockgroups * block_size));

	ReadBlock(2, buffer);
	for (i = 0; i < no_of_blockgroups; i++)
	{
		memcpy((void *) &group_descriptors[i],
				buffer + i * (long)sizeof(struct ext2_group_desc),
				sizeof(struct ext2_group_desc));
		ReadBlock((int)(group_descriptors[i].bg_block_bitmap),
				&block_bitmap[i * block_size]);
		ReadBlock((int)(group_descriptors[i].bg_inode_bitmap),
				&inode_bitmap[i * block_size]);
	}
}

//======================================================
// Return the value of the bit for block in the bitmpap
//======================================================
int GetBlockBitmapBit(int block)
{
	int group, block_in_group, byte_in_map, bit_in_byte;

	block--;
	group = block / sb.s_blocks_per_group;
	block_in_group = block % sb.s_blocks_per_group;
	byte_in_map = block_in_group / 8;
	bit_in_byte = block_in_group % 8;
	return (1 << bit_in_byte
			& *(block_bitmap + block_size * group + byte_in_map));
}

//======================================================
// Set the value of the bit for block in the bitmpap
//======================================================
void SetBlockBitmapBit(int block)
{
	int group, block_in_group, byte_in_map, bit_in_byte;

	block--;
	group = block / sb.s_blocks_per_group;
	block_in_group = block % sb.s_blocks_per_group;
	byte_in_map = block_in_group / 8;
	bit_in_byte = block_in_group % 8;
	*(block_bitmap + block_size * group + byte_in_map) |= 1 << bit_in_byte;
	group_descriptors[group].bg_free_blocks_count--;
	sb.s_free_blocks_count--;
}

//======================================================
// Clear the value of the bit for block in the bitmpap
//======================================================
void ClearBlockBitmapBit(int block)
{
	int group, block_in_group, byte_in_map, bit_in_byte;

	block--;
	group = block / sb.s_blocks_per_group;
	block_in_group = block % sb.s_blocks_per_group;
	byte_in_map = block_in_group / 8;
	bit_in_byte = block_in_group % 8;
	*(block_bitmap + block_size * group + byte_in_map) &= ~(1 << bit_in_byte);
	group_descriptors[group].bg_free_blocks_count++;
	sb.s_free_blocks_count++;
}

//=====================================================================
// Return the (absolute) block number of the first free block in group
//=====================================================================
int GetFreeBlock(int group)
{
	int block;
	int i = 0, j=0;
	while (block_bitmap[block_size * group + i] == -1)
		i++;
	for (j = 0; j < 8; j++)
		if (!(block_bitmap[i] & (1 << j)))
			break;
	block = group * sb.s_blocks_per_group + 8 * i + j +1;
	// Mark this block as in use
	SetBlockBitmapBit(block);
	return block;
}

//======================================================
// Return the value of the bit for inode in the bitmpap
//======================================================
int GetINodeBitmapBit(int inode)
{
	int group, inode_in_group, byte_in_map, bit_in_byte;

	inode--;
	group = inode / sb.s_inodes_per_group;
	inode_in_group = inode % sb.s_inodes_per_group;
	byte_in_map = inode_in_group / 8;
	bit_in_byte = inode_in_group % 8;
	return (1 << bit_in_byte
			& *(inode_bitmap + block_size * group + byte_in_map));
}

//======================================================
// Set the value of the bit for inode in the bitmpap
//======================================================
void SetINodeBitmapBit(int inode)
{
	int group, inode_in_group, byte_in_map, bit_in_byte;

	inode--;
	group = inode / sb.s_inodes_per_group;
	inode_in_group = inode % sb.s_inodes_per_group;
	byte_in_map = inode_in_group / 8;
	bit_in_byte = inode_in_group % 8;
	*(inode_bitmap + block_size * group + byte_in_map) |= 1 << bit_in_byte;
	group_descriptors[group].bg_free_inodes_count--;
	sb.s_free_inodes_count--;
}

//======================================================
// Clear the value of the bit for inode in the bitmpap
//======================================================
void ClearINodeBitmapBit(int inode)
{
	int group, inode_in_group, byte_in_map, bit_in_byte;

	inode--;
	group = inode / sb.s_inodes_per_group;
	inode_in_group = inode % sb.s_inodes_per_group;
	byte_in_map = inode_in_group / 8;
	bit_in_byte = inode_in_group % 8;
	*(inode_bitmap + block_size * group + byte_in_map) &= ~(1 << bit_in_byte);
	group_descriptors[group].bg_free_inodes_count++;
	sb.s_free_inodes_count++;
}

//=====================================================================
// Return the (absolute) inode number of the first free inode in group
//=====================================================================
int GetFreeINode(int group)
{
	int inode;
	int i = 0, j = 0;
	while (inode_bitmap[block_size * group + i] == -1)
		i++;
	for (j = 0; j < 8; j++)
		if (!(inode_bitmap[i] & (1 << j)))
			break;
	inode = group * sb.s_inodes_per_group + 8 * i + j + 1;
	// Mark this inode as in use
	SetINodeBitmapBit(inode);
	return inode;
}

//=====================================================================
// Flushes the cached values of the superblock, group descriptors, and
// block and inode bitmaps back to disk
//=====================================================================
void FlushCaches(void)
{
	// We need to test whether we are dealing with SPARSE_SUPERBLOCKS or not. Assume yes for the time being
	int i;
	char buffer[block_size];
	memset(buffer, 0, (size_t)block_size);

	for (i = 0; i < (int)no_of_blockgroups; i++)
		memcpy(buffer + i * sizeof(struct ext2_group_desc),
				(void *) &group_descriptors[i], sizeof(struct ext2_group_desc));

	for (i = 0; i < (int)no_of_blockgroups; i++)
	{
		// Write SuperBlock and Group Descriptors. Assume < 25 block_groups
		if (i == 0 || i == 1 || i == 3 || i == 5 || i == 7 || i == 9)
		{
			sb.s_block_group_nr = (u_int16_t)i;
			WriteBlock(i * block_size * sb.s_blocks_per_group + 1, (char *) &sb);
			WriteBlock(i * block_size * sb.s_blocks_per_group + 2, buffer);
		}
		WriteBlock((int)(group_descriptors[i].bg_block_bitmap),
				&block_bitmap[i * block_size]);
		WriteBlock((int)(group_descriptors[i].bg_inode_bitmap),
				&inode_bitmap[i * block_size]);
	}
}

//===================================================================
// Fill in the struct inode with the details from inode inodeNumber
//===================================================================
void GetINode(__le32 inodeNumber, struct ext2_inode *inode)
{
	int group_number, block_in_group, block, number_in_block;
	char buffer[block_size];
	struct ext2_inode *inodes;

	inodeNumber--;
	group_number = (int)(inodeNumber / sb.s_inodes_per_group);
	block_in_group = (inodeNumber % sb.s_inodes_per_group)
			/ inodes_per_block;
	block = (int)(group_descriptors[group_number].bg_inode_table + block_in_group);
	number_in_block = (inodeNumber % sb.s_inodes_per_group)
			% inodes_per_block;
	ReadBlock(block, buffer);
	inodes = (struct ext2_inode *) buffer;
	memcpy((void *) inode, (void *) &inodes[number_in_block],
			sizeof(struct ext2_inode));
}

//===================================================================
// Write the struct inode to the disk
//===================================================================
void PutINode(__le32 inodeNumber, struct ext2_inode *inode)
{
	int group_number, block_in_group, block, number_in_block;
	char buffer[block_size];
	struct ext2_inode *inodes;

	inodeNumber--;
	group_number = (int)(inodeNumber / sb.s_inodes_per_group);
	block_in_group = (inodeNumber % sb.s_inodes_per_group)
			/ inodes_per_block;
	block = (int)(group_descriptors[group_number].bg_inode_table + block_in_group);
	number_in_block = (inodeNumber % sb.s_inodes_per_group)
			% inodes_per_block;
	ReadBlock(block, buffer);
	inodes = (struct ext2_inode *) buffer;
	memcpy((void *) &inodes[number_in_block], (void *) inode,
			sizeof(struct ext2_inode));
	WriteBlock(block, buffer);
}

//===========================================================
// Returns the inode number of path or a negative error code
//===========================================================
__le32 GetFileINode(char *path)
{
	char *temp, *name;
	char buffer[block_size];
	int startingDirectory = currentDirectory;

	temp = AllocUMem((long)(strlen(path) + 1));
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
		ReadBlock((int)(parentINode.i_block[0]), (char *) &buffer);
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
				return 0;
			}
		}
		startingDirectory = (int)(dir->inode);
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
	if (fcb->bufferIsDirty)
		WriteBlock(fcb->currentBlock, fcb->buffer);
	SetIndexesFromCursor(fcb);
	fcb->currentBlock = fcb->inode->i_block[fcb->index1];
	ReadBlock(fcb->currentBlock, fcb->buffer);
	if (fcb->index1 >= EXT2_IND_BLOCK)
	{
		__le32 *blocks = (__le32 *) fcb->buffer;
		fcb->currentBlock = blocks[fcb->index2];
		ReadBlock(fcb->currentBlock, fcb->buffer);
	}
	if (fcb->index1 >= EXT2_DIND_BLOCK)
	{
		__le32 *blocks = (__le32 *) fcb->buffer;
		fcb->currentBlock = blocks[fcb->index3];
		ReadBlock(fcb->currentBlock, fcb->buffer);
	}
	if (fcb->index1 == EXT2_TIND_BLOCK)
	{
		__le32 *blocks = (__le32 *) fcb->buffer;
		fcb->currentBlock = blocks[fcb->index4];
		ReadBlock(fcb->currentBlock, fcb->buffer);
	}
}

//====================================================
// Add a block to a file that has no allocated blocks
//====================================================
void AddFirstBlockToFile(struct FCB *fcb)
{
	fcb->currentBlock = GetFreeBlock((int)(fcb->inodeNumber / sb.s_inodes_per_group));
	fcb->inode->i_block[0] = fcb->currentBlock;
	fcb->inode->i_blocks = 2;
}
//=======================================================================
// Add a new block the the file represented by fcb
// This is quite complicated because of the various level of indirection
//=======================================================================
void AddBlockToFile(struct FCB *fcb)
{
	char *buffer = AllocUMem(1024);
	int group = (int)(fcb->inodeNumber / sb.s_inodes_per_group);
	__le32 tempBlock, tempBlock2, tempBlock3;
	__le32 *blocks;


	memset(buffer, 0, 1024);
	fcb->currentBlock = GetFreeBlock(group);
	if (fcb->index1 < EXT2_IND_BLOCK)
	{
		fcb->index1++;
		if (fcb->index1 < EXT2_IND_BLOCK)
			// This is a direct block
			fcb->inode->i_block[fcb->index1] = fcb->currentBlock;
		else
		{
			// This is the first indirect block. Add a block for the indirect block table
			fcb->index2 = 0;
			tempBlock = fcb->inode->i_block[fcb->index1] = (u_int32_t)GetFreeBlock(
					group);
			// Add currentBlock as the first entry in the indirect block table
			blocks = (__le32 *) buffer;
			blocks[0] = fcb->currentBlock;
			WriteBlock(tempBlock, buffer);
		}
	}
	else if (fcb->index1 == EXT2_IND_BLOCK)
	{
		fcb->index2++;
		if (fcb->index2 < block_size / 32)
		{
			// This is an indirect block, so get the indirect block table
			ReadBlock((int)(fcb->inode->i_block[fcb->index1]), buffer);
			blocks = (__le32 *) buffer;
			// Add currentBlock to the indirect block table
			blocks[fcb->index2] = fcb->currentBlock;
			WriteBlock((int)(fcb->inode->i_block[fcb->index1]), buffer);
		}
		else
		{
			// This is the first double-indirect block. Add a block for the first-level indirect block table
			fcb->index1 = EXT2_DIND_BLOCK;
			fcb->index2 = 0;
			fcb->index3 = 0;
			tempBlock = fcb->inode->i_block[fcb->index1] = (u_int32_t)GetFreeBlock(
					group);
			// Add a block for the second-level indirect block table
			blocks = (__le32 *) buffer;
			tempBlock2 = blocks[0] = GetFreeBlock(group);
			WriteBlock(tempBlock, buffer);
			// Add current block to the indirect-indirect block table
			blocks[0] = fcb->currentBlock;
			WriteBlock(tempBlock2, buffer);
		}
	}
	else if (fcb->index1 == EXT2_DIND_BLOCK)
	{
		fcb->index3++;
		if (fcb->index3 < block_size / 32)
		{
			// This is a double-indirect block, so get the first-level indirect block table
			ReadBlock((int)(fcb->inode->i_block[fcb->index1]), buffer);
			// Get the second-level indirect block table for index2
			blocks = (__le32 *) buffer;
			tempBlock = blocks[fcb->index2];
			ReadBlock(tempBlock, buffer);
			//Add currentBlock to the double-indirect block table
			blocks[fcb->index3] = fcb->currentBlock;
			WriteBlock(tempBlock, buffer);
		}
		else
		{
			fcb->index3 = 0;
			fcb->index2++;
			if (fcb->index2 < block_size / 32)
			{
				// Still a double-indirect block, but we need to add another first-level table
				ReadBlock((int)(fcb->inode->i_block[fcb->index1]), buffer);
				blocks = (__le32 *) buffer;
				tempBlock = blocks[fcb->index2] = GetFreeBlock(group);
				WriteBlock((int)(fcb->inode->i_block[fcb->index1]), buffer);
				// Clear buffer
				memset(buffer, 0, (size_t)block_size);
				blocks[0] = fcb->currentBlock;
				WriteBlock(tempBlock, buffer);
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
				// Add a block for the second-level indirect block table
				blocks = (__le32 *) buffer;
				tempBlock2 = blocks[0] = GetFreeBlock(group);
				WriteBlock(tempBlock, buffer);
				// Add a block for the third-level indirect block table
				tempBlock3 = blocks[0] = GetFreeBlock(group);
				WriteBlock(tempBlock2, buffer);
				// Add current block to the indirect-indirect-indirect block table
				blocks[0] = fcb->currentBlock;
				WriteBlock(tempBlock3, buffer);
			}
		}
	}
	else if (fcb->index1 == EXT2_TIND_BLOCK)
	{
		// *** TO DO ***
		fcb->index4++;
	}
	fcb->inode->i_blocks += 2;
	fcb->inodeIsDirty = 1;
}
