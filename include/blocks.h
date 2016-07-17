/*
 * blocks.h
 *
 *  Created on: 6 Jun 2012
 *      Author: ian
 */
#include <ext2_fs.h>
#include <linux/types.h>
#include <kstructs.h>
#include <fat.h>

#ifndef BLOCKS_H_
#define BLOCKS_H_

/* inode: i_mode */
#define	EXT2_S_IFMT		0xF000	/* format mask  */
#define	EXT2_S_IFSOCK	0xC000	/* socket */
#define	EXT2_S_IFLNK	0xA000	/* symbolic link */
#define	EXT2_S_IFREG	0x8000	/* regular file */
#define	EXT2_S_IFBLK	0x6000	/* block device */
#define	EXT2_S_IFDIR	0x4000	/* directory */
#define	EXT2_S_IFCHR	0x2000	/* character device */
#define	EXT2_S_IFIFO	0x1000	/* fifo */

#define	EXT2_S_ISUID	0x0800	/* SUID */
#define	EXT2_S_ISGID	0x0400	/* SGID */
#define	EXT2_S_ISVTX	0x0200	/* sticky bit */
#define	EXT2_S_IRWXU	0x01C0	/* user access rights mask */
#define	EXT2_S_IRUSR	0x0100	/* read */
#define	EXT2_S_IWUSR	0x0080	/* write */
#define	EXT2_S_IXUSR	0x0040	/* execute */
#define	EXT2_S_IRWXG	0x0038	/* group access rights mask */
#define	EXT2_S_IRGRP	0x0020	/* read */
#define	EXT2_S_IWGRP	0x0010	/* write */
#define	EXT2_S_IXGRP	0x0008	/* execute */
#define	EXT2_S_IRWXO	0x0007	/* others access rights mask */
#define	EXT2_S_IROTH	0x0004	/* read */
#define	EXT2_S_IWOTH	0x0002	/* write */
#define	EXT2_S_IXOTH	0x0001	/* execute */

struct FCB
{
	struct ext2_inode *inode;
	unsigned short pid;
	u_int32_t inodeNumber;
	u_int32_t currentBlock;
	int index1, index2, index3, index4;
	int fileCursor;
	int bufCursor;
	char *buffer;
	char deviceType;
	char bufferIsDirty;
	char inodeIsDirty;
	unsigned char openCount;
	long (*read)(struct FCB *, char *, long);
	long (*write)(struct FCB *, char *, long);
	void (*close)(struct FCB *);
};

// These are the numbers of blocks pointed to by inode->i_block[]
#define SECTOR_SIZE	512
#define INODE_BLOCK_SIZE 512

#define DIRECT_BLOCKS			12
#define INDIRECT_BLOCKS			(block_size / sizeof(__le32))
#define DOUBLE_INDIRECT_BLOCKS	INDIRECT_BLOCKS * INDIRECT_BLOCKS
#define I_BLOCK_SIZE_RATIO			(block_size / INODE_BLOCK_SIZE)

void ReadBlock(u_int32_t block, char *buffer);
void WriteBlock(u_int32_t block, char *buffer);
void InitializeHD();
__le32 GetFileINode(char *path);
void GetINode(u_int32_t inode_number, struct ext2_inode *inode);
void PutINode(u_int32_t inode_number, struct ext2_inode *inode);
u_int32_t GetFreeINode(u_int32_t group);
void SetBufferFromCursor(struct FCB *fcb);
void AddFirstBlockToFile(struct FCB *fcb);
void AddBlockToFile(struct FCB *fcb);
void FlushCaches(void);
void ClearBlockBitmapBit(u_int32_t block);
void ClearINodeBitmapBit(u_int32_t inode);

#endif /* BLOCKS_H_ */
