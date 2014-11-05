//Primary author: Jonathan Bedard
//Confirmed working: 9/26/2014

//This class is the header file for the generalized AVL tree

#ifndef AVL_H
#define AVL_H

//The AVL Node class
template <class dataType>
class AVLNode
{
protected:
	AVLNode* parent;
	AVLNode** children;
	dataType* data;
	bool traverse_flag;
	int height;

public:
	//Constructor/Desturctor
	AVLNode(dataType* d);
	virtual ~AVLNode();
	void deleteData();
	int compare(AVLNode<dataType>* inp);

	//Get Functions
	dataType* getData();
	AVLNode <dataType>* getParent();
	AVLNode <dataType>* getChild(int x);
	bool getFlag();
	int getHeight();

	//Set Functions
	void setHeight();
	void setChild(AVLNode<dataType>* c);
	void setParent(AVLNode<dataType>* p);
	void removeChild(AVLNode<dataType>* c);
	void removeChild(int pos);
	void removeParent();

	//Linear Traversal
	void resetTraverse();
	AVLNode <dataType>* getNext();
	AVLNode <dataType>* getPrev();


};

//The AVL Tree Class
template <class dataType>
class AVLTree
{
protected:
	AVLNode <dataType>* root;
	int numElements;

	//Private Rotation and balancing
	bool balanceDelete(AVLNode <dataType>* x);
	bool checkBalance(AVLNode <dataType>* x);
	void balanceUp(AVLNode <dataType>* x);
	bool balance(AVLNode <dataType>* x);
	bool singleRotation(AVLNode <dataType>* r, int dir);
	bool doubleRotation(AVLNode <dataType>* r, int dir);
	AVLNode <dataType>* findBottom(AVLNode<dataType>* x, int dir);

public:
	//Constructors
	AVLTree();
	virtual ~AVLTree();
	void deleteData();

	//Get Functions
	AVLNode <dataType>* getRoot();
	AVLNode <dataType>* find(dataType* x);
	AVLNode <dataType>* find(AVLNode<dataType>* x);
	bool insert(dataType* x);

	//Rotation and Balancing
	bool findDelete(dataType* x);
	bool findDelete(AVLNode <dataType>* x);
	

	//Linear Traversal
	void resetTraverse();
	AVLNode <dataType>* getFirst();
	AVLNode <dataType>* getLast();
};

//Allows for automatic inclusion of AVL_CPP
#ifndef AVL_CPP
#include "AVL.cpp"
#endif

#endif