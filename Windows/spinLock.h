//Primary author: Jonathan Bedard
//Confirmed working: 10/29/2014

/*
WINDOWS ONLY
*/

#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <atomic>

using namespace std;

//This is a simple spinlock class
class spinLock
{
private:
    atomic_flag lock;
	bool taken;
public:
	spinLock();
	~spinLock();
    void acquire();
    void release();
	bool isTaken();
};

#endif