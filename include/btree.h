/*
 * btree.h
 *
 *  Created on: Sep 7, 2011
 *      Author: ian
 */

#ifndef BTREE_H_
#define BTREE_H_

struct BTreeNode
{
	void * data;
	struct BTreeNode * lesser;	// Tree of nodes whose key is less than this one
	struct BTreeNode * greater;	// Tree of nodes whose key is greater than this one
	int timestamp; 				// Value of Ticks when the node was created.
	int key;
	unsigned char isDirty; 		// Has data been written to the buffer since it was loaded?
};

struct BTreeNode * CreateBTreeNode(int key, void * data);
void AddBTreeNode(struct BTreeNode * btree, int key, void *data);
struct BTreeNode * FindBTreeNode(struct BTreeNode * btree, int key);
struct BTreeNode * DeleteBTreeNode(struct BTreeNode * btree, int key);
struct BTreeNode * BalanceBTree(struct BTreeNode * btree);

#endif /* BTREE_H_ */
