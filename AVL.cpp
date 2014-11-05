//Primary author: Jonathan Bedard
//Confirmed working: 10/13/2014

#ifndef AVL_CPP
#define AVL_CPP

#include "AVL.h"

#include <stdio.h>
#include <iostream>

using namespace std;

//AVL Tree ----------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Initializers-------------------------------------------------------------------------------------------------------------------------------------------------------------

	//Constructor
	template <class dataType>
	AVLTree<dataType>::AVLTree()
	{
		numElements = 0;
		root = NULL;
	}
	//Destructor
	template <class dataType>
	AVLTree<dataType>::~AVLTree()
	{
		if(root!=NULL)
			delete root;
	}
	//Destructor
	template <class dataType>
	void AVLTree<dataType>::deleteData()
	{
		if(root!=NULL)
			root->deleteData();
	}
	//Insertion function
	template <class dataType>
	bool AVLTree<dataType>::insert(dataType* x)
	{
		AVLNode<dataType>* newNode = new AVLNode<dataType>(x);
		AVLNode<dataType>* trc = root;
		AVLNode<dataType>* prev = NULL;
		int temp;

		
		while(trc != NULL)
		{
			prev = trc;
			temp = trc->compare(newNode);

			//Return that the input has already been inserted
			if(temp == 0)
				return false;
			//Iterate down the tree
			else if(temp < 0)
			{
				trc = trc->getChild(0);
			}
			else
			{
				trc = trc->getChild(1);
			}
		}

		//Put the new node into the list
		newNode->setParent(prev);
		newNode->setHeight();
		if(root == NULL)
			root = newNode;

		balanceUp(newNode);
		//Return a valid insert
		numElements++;
		return true;
	}

//Get Functions-----------------------------------------------------------------------------------------------------------------------------------------------------------

	//Returns the root of the tree
	template <class dataType>
	AVLNode<dataType>* AVLTree<dataType>::getRoot()
	{
		return root;
	}
	//Returns a data points node on the tree
	template <class dataType>
	AVLNode <dataType>* AVLTree<dataType>::find(dataType* x)
	{
		AVLNode <dataType>* d = new AVLNode<dataType> (x);
		AVLNode <dataType>* tem = find(d);
		delete d;
		return tem;
	}
	//Returns the actual position node
	template <class dataType>
	AVLNode <dataType>* AVLTree<dataType>::find(AVLNode<dataType>* x)
	{
		AVLNode<dataType>* trc = root;
		while(trc != NULL)
		{
			if(trc->compare(x) == 0)
				return trc;
			else if(trc->compare(x)< 0)
				trc = trc->getChild(0);
			else
				trc = trc->getChild(1);
		}
		return NULL;
	}

//Rotation and Balancing--------------------------------------------------------------------------------------------------------------------------------------------------

	//Deletes a Node, given data
	template <class dataType>
	bool AVLTree<dataType>::findDelete(dataType* x)
	{
		AVLNode <dataType>* d = new AVLNode<dataType> (x);
		bool tem = findDelete(d);
		delete d;
		return tem;
	}
	//Deletes a node, given a node
	template <class dataType>
	bool AVLTree<dataType>::findDelete(AVLNode <dataType>* x)
	{
		AVLNode <dataType>* toBeDel = find(x);
		if(toBeDel == NULL)
			return false;
		return balanceDelete(toBeDel);
	}
	//Deletes a node and balances the tree
	template <class dataType>
	bool AVLTree<dataType>::balanceDelete(AVLNode <dataType>* x)
	{
		if(x == NULL)
			return false;

		//The case where the node has no children
		if(x->getChild(0) == NULL && x->getChild(1) == NULL)
		{
			if(x == root)
			{
				root = NULL;
				numElements--;
				return true;
			}
			if(x->getParent() != NULL)
				x->getParent()->removeChild(x);
			else
				return false;
			balanceUp(x->getParent());
			numElements--;
			return true;
		}

		//Check children
		int bal = 0;

		if(x->getChild(0) != NULL)
			bal = x->getChild(0)->getHeight()+1;
		if(x->getChild(1) != NULL)
			bal = bal-(x->getChild(1)->getHeight()+1);

		AVLNode <dataType>* newTop;
		//The 0 child is taller
		if(bal > 0)
		{
			newTop = findBottom(x->getChild(0),1);
			bal = 0;
		}
		//The 1 child is taller
		else
		{
			newTop = findBottom(x->getChild(1),0);
			bal = 1;
		}
		if(newTop == NULL)
			return false;

		AVLNode <dataType>* rebal;
		if(newTop->getChild(bal) == NULL)
			rebal= newTop->getParent();
		else
			rebal=newTop->getChild(bal);
		if(rebal == NULL)
			return false;

		if(newTop->getChild(bal) != NULL)
			newTop->getParent()->setChild(rebal);
		else
			newTop->getParent()->removeChild(newTop);

		if(x->getParent() != NULL)
		{
			newTop->setParent(x->getParent());
			x->getParent()->removeChild(x);
		}
		else
		{
			if(x == root)
			{
				root = newTop;
				newTop->setParent(NULL);
			}
			else
				return false;
		}
		if(x->getChild(0) != NULL)
			x->getChild(0)->setParent(newTop);
		if(x->getChild(1) != NULL)
			x->getChild(1)->setParent(newTop);

		newTop->setHeight();
		if(rebal != x)
		{
			rebal->setHeight();
			balanceUp(rebal);
		}
		else
		{
			balanceUp(newTop);
		}

		numElements--;
		return true;
	}
	//Detect balance
	template <class dataType>
	bool AVLTree<dataType>::checkBalance(AVLNode <dataType>* x)
	{
		if(x == NULL)
			return true;

		int bal = 0;

		if(x->getChild(0) != NULL)
			bal = x->getChild(0)->getHeight()+1;
		if(x->getChild(1) != NULL)
			bal = bal-(x->getChild(1)->getHeight()+1);

		if(!(bal==-1 || bal==0 || bal==1))
			return false;
		return true;
	}
	//Balance up the tree
	template <class dataType>
	void AVLTree<dataType>::balanceUp(AVLNode <dataType>* x)
	{
		if(x == NULL)
			return;
		AVLNode <dataType>* origPar = x->getParent();
		balance(x);
		if(origPar != NULL)
			balanceUp(origPar);
	}
	//Balance a single node
	template <class dataType>
	bool AVLTree<dataType>::balance(AVLNode <dataType>* x)
	{
		if(checkBalance(x) == true)
			return true;

		int test0 = 0;
		int test1 = 0;

		if(x->getChild(0) != NULL)
			test0 = x->getChild(0)->getHeight() + 1;
		if(x->getChild(1) != NULL)
			test1 = x->getChild(1)->getHeight() + 1;

		int hld1;

		if(test0 > test1)
			hld1 = 0;
		else
			hld1 = 1;

		test0 = 0;
		test1 = 0;

		if(x->getChild(hld1)->getChild(hld1) != NULL)
			test0 = x->getChild(hld1)->getChild(hld1)->getHeight() +1;
		if(x->getChild(hld1)->getChild((hld1+1)%2) != NULL)
			test1 = x->getChild(hld1)->getChild((hld1+1)%2)->getHeight()+1;

		if(test0 > test1)
			singleRotation(x,hld1);
		else
			doubleRotation(x,hld1);

		return false;
	}
	//Single rotation
	template <class dataType>
	bool AVLTree<dataType>::singleRotation(AVLNode <dataType>* r, int dir)
	{
		//Check valid inputs
		if(!(dir == 1 || dir == 0))
			return false;
		if(r == NULL)
			return false;
		AVLNode <dataType>* save = r->getChild(dir);
		if(save == NULL)
			return false;

		//Preform rotation
		save->setParent(r->getParent());
		if(save->getChild((dir+1)%2) != NULL)
			save->getChild((dir+1)%2)->setParent(r);
		else
			r->removeChild(dir);
		r->setParent(save);

		//Re-check height
		r->setHeight();

		//Reassign root
		if(root == r)
			root = save;
		return true;
	}
	//Double rotation
	template <class dataType>
	bool AVLTree<dataType>::doubleRotation(AVLNode <dataType>* r, int dir)
	{
		//Check valid inputs
		if(!(dir == 1 || dir == 0))
			return false;
		if(r == NULL)
			return false;
		AVLNode<dataType>* save1 = r->getChild(dir);
		if(save1 == NULL)
			return false;
		AVLNode<dataType>* save2 = save1->getChild((dir+1)%2);
		if(save2 == NULL)
			return false;

		if(save2->getChild(dir) != NULL)
			save2->getChild(dir)->setParent(save1);
		else
			save1->removeChild((dir+1)%2);
		
		save2->setParent(r);
		save1->setParent(save2);

		bool temp = singleRotation(r,dir);
		save1->setHeight();
		return temp;
	}
	//Finds the last node in a tree in a specific direction
	template <class dataType>
	AVLNode<dataType>* AVLTree<dataType>::findBottom(AVLNode<dataType>* x, int dir)
	{
		if(dir!=0&&dir!=1)
			return NULL;
		AVLNode<dataType>* trc = x;
		AVLNode<dataType>* hld = trc;

		while(trc != NULL)
		{
			hld = trc;
			trc=trc->getChild(dir);
		}
		return hld;
	}

//Traversal Functions-----------------------------------------------------------------------------------------------------------------------------------------------------

	//Resets the traversal variables
	template <class dataType>
	void AVLTree<dataType>::resetTraverse()
	{
		if(root != NULL)
			root->resetTraverse();
	}
	//Returns the first element
	template <class dataType>
	AVLNode<dataType>* AVLTree<dataType>::getFirst()
	{
		AVLNode<dataType>* trc = root;
		if(trc==NULL)
			return NULL;
		while(trc->getChild(1)!=NULL)
		{
			trc = trc->getChild(1);
		}
		return trc;
	}
	//Returns the last element
	template <class dataType>
	AVLNode<dataType>* AVLTree<dataType>::getLast()
	{
		AVLNode<dataType>* trc = root;
		if(trc==NULL)
			return NULL;
		while(trc->getChild(0)!=NULL)
		{
			trc = trc->getChild(0);
		}
		return trc;
	}


//AVL Node ----------------------------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Initializers-------------------------------------------------------------------------------------------------------------------------------------------------------------

	//Constructor
	template <class dataType>
	AVLNode<dataType>::AVLNode(dataType* d)
	{
		children = new AVLNode*[2];
		children[0] = NULL;
		children[1] = NULL;

		data = d;
		traverse_flag = false;
		height = 0;
	}
	//Destructor
	template <class dataType>
	AVLNode<dataType>::~AVLNode()
	{
		if(children[0] != NULL)
			delete children[0];
		if(children[1] != NULL)
			delete children[1];
		delete children;
	}
	//Deletes the data
	template <class dataType>
	void AVLNode<dataType>::deleteData()
	{
		delete data;
		if(children[0]!=NULL)
			children[0]->deleteData();
		if(children[1]!=NULL)
			children[1]->deleteData();
	}
	//Compares two AVL Nodes
	template <class dataType>
	int AVLNode<dataType>::compare(AVLNode<dataType>* inp)
	{
		if(inp==NULL)
			return 1;
		if((*data)==(*(inp->getData())))
			return 0;
		if((*data)>(*(inp->getData())))
			return 1;
		return -1;
	}

//Get Functions-----------------------------------------------------------------------------------------------------------------------------------------------------------

	//Returns the data
	template <class dataType>
	dataType* AVLNode<dataType>::getData()
	{
		return data;
	}
	//Returns the parent
	template <class dataType>
	AVLNode <dataType>* AVLNode<dataType>::getParent()
	{
		return parent;
	}
	//Returns the child
	template <class dataType>
	AVLNode <dataType>* AVLNode<dataType>::getChild(int x)
	{
		if(x!=0 && x!=1)
			return NULL;
		return children[x];
	}
	//Returns the traversal flag
	template <class dataType>
	bool AVLNode<dataType>::getFlag()
	{
		return traverse_flag;
	}
	//Returns the height of the tree at this node
	template <class dataType>
	int AVLNode<dataType>::getHeight()
	{
		return height;
	}

//Set Functions-----------------------------------------------------------------------------------------------------------------------------------------------------------

	//Sets the height of the tree
	template <class dataType>
	void AVLNode<dataType>::setHeight()
	{
		int temp;

		if(children[0] == NULL)
			temp = -1;
		else
			temp = children[0]->getHeight();

		if(children[1]!=NULL)
			if(children[1]->getHeight()>temp)
				temp = children[1]->getHeight();

		if(height == temp +1 && height != 0)
			return;
		height = temp+1;

		if(parent!=NULL)
			parent->setHeight();
	}
	//Sets the Child
	template <class dataType>
	void AVLNode<dataType>::setChild(AVLNode<dataType>* c)
	{
		if(c!=NULL)
		{
			if(compare(c) < 0)
				children[0] = c;
			else
				children[1] = c;
		}
	}
	//Sets the parent
	template <class dataType>
	void AVLNode<dataType>::setParent(AVLNode<dataType>* p)
	{
		parent = p;
		if(p!=NULL)
			p->setChild(this);
	}
	//Removes the given child
	template <class dataType>
	void AVLNode<dataType>::removeChild(AVLNode<dataType>* c)
	{
		if(children[0]!=NULL&&children[0]->compare(c)==0)
			children[0] = NULL;
		if(children[1]!=NULL&&children[1]->compare(c)==0)
			children[1] = NULL;
	}
	//Removes the child in the given location
	template <class dataType>
	void AVLNode<dataType>::removeChild(int pos)
	{
		if(pos!=0 && pos!=1)
			return;
		children[pos] = NULL;
	}
	//Removes the parent
	template <class dataType>
	void AVLNode<dataType>::removeParent()
	{
		parent = NULL;
	}

//Traversal Functions-----------------------------------------------------------------------------------------------------------------------------------------------------

	//Resets the traversal variables
	template <class dataType>
	void AVLNode<dataType>::resetTraverse()
	{
		traverse_flag = false;

		if(children[0] != NULL)
			children[0]->resetTraverse();
		if(children[1] != NULL)
			children[1]->resetTraverse();
	}
	//Returns the next element
	template <class dataType>
	AVLNode <dataType>* AVLNode<dataType>::getNext()
	{
		traverse_flag=true;

		AVLNode<dataType>* parTrc = children[0];

		//Return smallest right descendent
		if(children[0]!=NULL)
		{
			while(parTrc->getChild(1)!=NULL)
				parTrc=parTrc->getChild(1);
			return parTrc;
		}

		//Check parent
		parTrc = parent;
		if(parTrc!=NULL)
		{
			while(parTrc!=NULL&&parTrc->getFlag())
				parTrc=parTrc->getParent();
			return parTrc;
		}
		return NULL;
	}
	//Returns the previous element
	template <class dataType>
	AVLNode <dataType>* AVLNode<dataType>::getPrev()
	{
		traverse_flag=true;

		AVLNode<dataType>* parTrc = children[1];

		//Return smallest left descendent
		if(children[1]!=NULL)
		{
			while(parTrc->getChild(0)!=NULL)
				parTrc=parTrc->getChild(0);
			return parTrc;
		}

		//Check parent
		parTrc = parent;
		if(parTrc!=NULL)
		{
			while(parTrc!=NULL&&parTrc->getFlag())
				parTrc=parTrc->getParent();
			return parTrc;
		}
		return NULL;
	}

#endif