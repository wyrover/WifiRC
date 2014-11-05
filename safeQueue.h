//Primary author: Jonathan Bedard
//Confirmed working: 9/26/2014

//This is the thread-safe queue

#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include "spinLock.h"

//The safeQueue class
template <class dataType>
class safeQueue
{
private:
	int size;
	dataType** array;
	int start;
	int end;

	spinLock lock;
public:
	safeQueue(int s);
	safeQueue();
	~safeQueue();
	void push(dataType* x);
	dataType* pop();
	bool empty();
};

//Constructor
template <class dataType>
safeQueue<dataType>::safeQueue(int s)
{
	//Set the size of the queue
	if(s>0)
		size = s;
	else
		s = 10;

	//Build the queue array
	array = new dataType*[size];

	start = 0;
	end = 0;
}
//Constructor
template <class dataType>
safeQueue<dataType>::safeQueue()
{
	//Set the size of the queue
	size = 40;

	//Build the queue array
	array = new dataType*[size];

	start = 0;
	end = 0;

}
//Destructor
template <class dataType>
safeQueue<dataType>::~safeQueue()
{
	lock.acquire();
	delete[] array;
	lock.release();
}
//Push into the queue
template <class dataType>
void safeQueue<dataType>::push(dataType* x)
{
	lock.acquire();

	if(array==NULL)
		return;

	if((start+1)%size==end)
	{
		delete(array[end]);
		end++;
	}

	//Move the trace pointer
	start=(start+1)%size;

	//Place into
	array[(start+size-1)%size]=x;

	lock.release();
}
//Pop from the queue
template <class dataType>
dataType* safeQueue<dataType>::pop()
{
	lock.acquire();

	if(empty())
		return NULL;

	//Move the trace pointer
	end = (end+1)%size;

	//Pop the element
	dataType* ret = array[(end+size-1)%size];

	lock.release();

	return ret;
}
//Check if the queue is empty
template <class dataType>
bool safeQueue<dataType>::empty()
{
	if(array==NULL)
		return true;
	return (start==end);
}

#endif
