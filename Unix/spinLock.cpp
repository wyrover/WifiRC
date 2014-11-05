//Primary author: Jonathan Bedard
//Confirmed working: 11/2/2014

/*
UNIX ONLY
*/

#ifndef SPINLOCK_CPP
#define SPINLOCK_CPP

#include "spinLock.h"
#include <pthread.h>

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
	pthread_spin_lock(&spinlock);
	taken = true;
}
//Release the lock
void spinLock::release()
{
	pthread_spin_unlock(&spinlock);
	taken = false;
}
//Test the lock (non blocking)
bool spinLock::isTaken()
{
	return taken;
}

#endif
