//Primary author: Jonathan Bedard
//Confirmed working: 10/23/2014

#ifndef SERIALTHREAD_CPP
#define SERIALTHREAD_CPP

#include "WifiRC.h"
#include <string>
#include <iostream>
#include <stdio.h>

using namespace std;

//Static Functions-------------------------------------------------------------------------------------------
	//Spawns the search thread
	static void serialSearch(void* ptr)
	{
		serialThread * stX = (serialThread*)ptr;
		stX->serialLoop();
	}

//Constructor/Destructor-------------------------------------------------------------------------------------
	//Constructor
	serialThread::serialThread()
	{
		connection = NULL;
		conName = NULL;
		nameList=NULL;
		build(false);
	}
	//Track constructor
	serialThread::serialThread(bool track)
	{
		connection = NULL;
		conName = NULL;
		nameList=NULL;
		build(track);
	}
	//Builds the serialThread
	void serialThread::build(bool track)
	{
		print = track;
		active = true;
		conName = NULL;
		connection=NULL;

		//Initialize the reset test
		resetTest = 0;

		//Initialize the valid names
		numNames = 8;
		if(nameList!=NULL)
			delete[] nameList;
		nameList = new string[numNames];

		nameList[0] = "COM2";
		nameList[1] = "COM3";
		nameList[2] = "COM4";
		
		nameList[3] = "/dev/ttyUSB0";
		nameList[4] = "/dev/ttyUSB1";
		nameList[5] = "/dev/ttyUSB2";
		nameList[6] = "/dev/ttyUSB3";
		nameList[7] = "/dev/ttyUSB4";

		if(print)
			cout<<"Serial Thread Begun"<<endl;
		spawnThread(&serialSearch,this);
	}
	//Destructor
	serialThread::~serialThread()
	{
		if(print)
			cout<<"Deleteing serial connections"<<endl;

		//Kill the thread
		active = false;

		sleep(250);

		//Delete the serial connection
		if(connection!=NULL)
			delete connection;

		//Free the name list
		if(nameList!=NULL)
			delete[] nameList;

		//Free the connection name
		if(conName!=NULL)
			delete conName;

		//Set to NULL for the thread to catch
		connection = NULL;
		conName = NULL;
		nameList=NULL;
	}

//Connection Functions---------------------------------------------------------------------------------------
	//The generalized (and public) loop function
	void serialThread::serialLoop()
	{
		while(active)
		{
			if(resetTest>10)
			{
				delete connection;
				connection = NULL;
			}
			search();
		}
	}
	//The establish serial connection function
	void serialThread::search()
	{
		int cnt = 0;
		Serial* temp=NULL;
		while(active&&(connection==NULL||!connection->IsConnected()))
		{
			cnt = 0;
			while(cnt<numNames&&active&&(connection==NULL||!connection->IsConnected()))
			{
				temp = new Serial((char*)nameList[cnt].c_str(),false);
				if(temp->IsConnected())
				{
					connection=temp;
					resetTest=0;
				}
				else
					delete temp;
				cnt++;
			}
		}
	}
	//Sending data
	void serialThread::sendData(uint8_t * x, unsigned int nb)
	{
		if(connection!=NULL&&connection->IsConnected())
			if (connection->WriteData(x,nb))
				resetTest=0;
			else
				resetTest++;
	}

#endif