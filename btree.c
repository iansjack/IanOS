/* btree.c
 *
 *  Created on: Sep 7, 2011
 *      Author: ian
 */

#include <btree.h>
#include <memory.h>

extern long Ticks;

//===================================================================
// Create a new node in isolation
// Returns a pointer to the new node
//===================================================================
struct BTreeNode * CreateBTreeNode(int key, void * data)
{
	struct BTreeNode * node = (struct BTreeNode *)AllocUMem(sizeof(struct BTreeNode));
	node->key = key;
	node->data = data;
	node->lesser = 0;
	node->greater = 0;
	node->isDirty = 0;
	node->timestamp = Ticks;
	return node;
}

//===================================================================
// Create a new node and add it to a tree
//===================================================================
void AddBTreeNode(struct BTreeNode * btree, int key, void *data)
{
	if (key > btree->key)
	{
		if (btree->greater)
			AddBTreeNode(btree->greater, key, data);
		else
			btree->greater = CreateBTreeNode(key, data);
	}
	else
	{
		if (btree->lesser)
			AddBTreeNode(btree->lesser, key, data);
		else
			(btree->lesser = CreateBTreeNode(key, data));
	}
}

//===================================================================
// Find a node in the tree given its key
// Returns a pointer to the found node
// Returns 0 if the node is not found
//===================================================================
struct BTreeNode * FindBTreeNode(struct BTreeNode * btree, int key)
{
	if (btree)
	{
		if (btree->key == key)
			return btree;
		else if (key > btree->key)
			return FindBTreeNode(btree->greater, key);
		else
			return FindBTreeNode(btree->lesser, key);
	}
	else
		return 0;
}

//===================================================================
// Delete a node (identified by key) from a tree
// Returns a pointer to the tree (which may differ from the original)
//===================================================================
struct BTreeNode * DeleteBTreeNode(struct BTreeNode * btree, int key)
{
	/*	struct BTreeNode * retVal = btree;
	 struct BTreeNode * tempVal = 0;

	 if (key == btree->key)
	 {
	 if (btree->noLesser > btree->noGreater)
	 {
	 retVal = btree->lesser;
	 tempVal = btree->lesser;
	 while (tempVal->greater)
	 {
	 tempVal->noGreater += btree->noGreater;
	 tempVal = tempVal->greater;
	 }
	 tempVal->greater = btree->greater;
	 tempVal->noGreater = btree->noGreater;
	 }
	 else
	 {
	 retVal = btree->greater;
	 tempVal = btree->greater;
	 while (tempVal->lesser)
	 {
	 tempVal->noLesser += btree->noLesser;
	 tempVal = tempVal->lesser;
	 }
	 tempVal->noLesser = btree->noLesser;
	 tempVal->lesser = btree->lesser;
	 }
	 free(btree);
	 }
	 else
	 {
	 if (key > btree->key)
	 {
	 tempVal = DeleteBTreeNode(btree->greater, key);
	 if (tempVal)
	 {
	 btree->greater = tempVal;
	 btree->noGreater--;
	 }
	 }
	 else
	 {
	 tempVal = DeleteBTreeNode(btree->lesser, key);
	 if (tempVal)
	 {
	 btree->lesser = tempVal;
	 btree->noLesser--;
	 }
	 }
	 retVal = btree;
	 }
	 */
	return btree;
}

int TreeSize(struct BTreeNode * btree)
{
	int size = 1;

	if (!btree)
		return 0;
	size += TreeSize(btree->lesser);
	size += TreeSize(btree->greater);
	return size;
}

//===================================================================
// Balances a tree
// Returns a pointer to the tree (which may differ from the original)
//===================================================================
struct BTreeNode * BalanceBTree(struct BTreeNode * btree)
{
	int difference;
	int nLesser = TreeSize(btree->lesser);
	int nGreater = TreeSize(btree->greater);

	difference = nLesser - nGreater;
	if (difference < 0)
		difference = -difference;
	while (difference > 1)
	{
		struct BTreeNode *tempNode, *temp2Node;
		if (nLesser < nGreater)
		{
			// Make the smallest node in Greater the new tree root
			// Get the node
			tempNode = btree->greater;
			if (!tempNode->lesser)
			{
				tempNode->lesser = btree;
			}
			else
			{
				while (tempNode->lesser)
				{
					temp2Node = tempNode;
					tempNode = tempNode->lesser;
				}
				temp2Node->lesser = 0;
				tempNode->lesser = btree;
				temp2Node = tempNode;
				while (temp2Node->greater)
					temp2Node = temp2Node->greater;
				temp2Node->greater = btree->greater;
			}
			btree->greater = 0;
			btree = tempNode;
		}
		else
		{
			// Make the biggest node in Lesser the new tree root
			tempNode = btree->lesser;
			if (!tempNode->greater)
			{
				tempNode->greater = btree;
			}
			else
			{
				while (tempNode->greater)
				{
					temp2Node = tempNode;
					tempNode = tempNode->greater;
				}
				temp2Node->greater = 0;
				tempNode->greater = btree;
				temp2Node = tempNode;
				while (temp2Node->lesser)
					temp2Node = temp2Node->lesser;
				temp2Node->lesser = btree->lesser;
			}
			btree->lesser = 0;
			btree = tempNode;
		}
		difference -= 2;
	}
	if (btree->lesser)
		btree->lesser = BalanceBTree(btree->lesser);
	if (btree->greater)
		btree->greater = BalanceBTree(btree->greater);
	return btree;
}
