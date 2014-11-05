//Primary author: Jonathan Bedard
//Confirmed working: 9/26/2014

#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <stdint.h>
#include "Serial.h"

class serialThread
{
private:
	Serial* connection;
	char* conName;
	string* nameList;
	int numNames;
	int resetTest;
	bool print;
	bool active;
	void build(bool track);

	//Connection Management
	void search();
	void listen();

public:
	serialThread();
	serialThread(bool track);
	~serialThread();

	//Connectiong Management
	void serialLoop();
	void sendData(uint8_t * x, unsigned int nb);

};

#endif