//Primary author: Jonathan Bedard
//Confirmed working: 11/2/2014

/*
WINDOWS ONLY
*/

#ifndef SPINLOCK_CPP
#define SPINLOCK_CPP

#include "spinLock.h"
#include <process.h>
#include <Windows.h>

using namespace std;

//Default constructor
spinLock::spinLock()
{
	release();
}
//Default destructor
spinLock::~spinLock(){}
//Acquire the lock
void spinLock::acquire()
{
	while (lock.test_and_set(memory_order_acquire))
	{Sleep(0);}
	taken = true;
}
//Release the lock
void spinLock::release()
{
	lock.clear(memory_order_release);
	taken = false;
}
//Test the lock (non blocking)
bool spinLock::isTaken()
{
	return taken;
}
#endif
