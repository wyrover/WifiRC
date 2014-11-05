//Primary author: Jonathan Bedard
//Confirmed working: 11/2/2014

/*
UNIX ONLY
*/

#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <pthread.h>

using namespace std;

//This is a simple spinlock class
class spinLock
{
private:
    pthread_spinlock_t spinlock;
	bool taken;
public:
	spinLock();
	~spinLock();
    void acquire();
    void release();
	bool isTaken();
};

#endif
