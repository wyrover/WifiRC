//Primary author: Jonathan Bedard
//Confirmed working: 10/27/2014

#ifndef RCCONTROLS_CPP
#define RCCONTROLS_CPP

#include "WifiRC.h"
#include <stdint.h>
#include <string>
#include <iostream>
#include <stdio.h>

using namespace std;

//Constructor-------------------------------------------------------------
	//The RCControler constructer
	RCControls::RCControls():
		GenControls(5,1)
	{
		mainPower=0;
		turnAngle=0;
		powerBound = 100;
		boundAngle = 50;

		setOn(true);
	}
	//The RCController destructor
	RCControls::~RCControls()
	{
	}

//Get Functions-----------------------------------------------------------

	//Returns the power value
	int8_t RCControls::getPower()
	{
		return mainPower;
	}
	//Returns the angle value
	int8_t RCControls::getAngle()
	{
		return turnAngle;
	}
	//Returns if the machine is on
	bool RCControls::getOn()
	{
		return getControl(4);
	}
	//Returns the forward bit
	bool RCControls::getForward()
	{
		return getControl(0);
	}
	//Returns the left bit
	bool RCControls::getLeft()
	{
		return getControl(1);
	}
	//Returns the backward bit
	bool RCControls::getBackward()
	{
		return getControl(2);
	}
	//Returns the right bit
	bool RCControls::getRight()
	{
		return getControl(3);
	}

//Get Command Functions---------------------------------------------------

	//Returns if a picture should be taken
	bool RCControls::getPicture()
	{
		return getCommand(0);
	}

//Set Functions-----------------------------------------------------------

	//Sets the angle
	void RCControls::setPower(int x)
	{
		if(x>powerBound)
			mainPower=powerBound;
		else if(x<-powerBound)
			mainPower=-powerBound;
		else
			mainPower = (int8_t) x;
	}
	//Sets the power
	void RCControls::setAngle(int x)
	{
		if(x>boundAngle)
			turnAngle=boundAngle;
		else if(x<-boundAngle)
			turnAngle=-boundAngle;
		else
			turnAngle = (int8_t) x;
	}
	//Sets the on command
	void RCControls::setOn(bool x)
	{
		setControl(4,x);
	}
	//Sets the forward control
	void RCControls::setForward(bool x)
	{
		setControl(0,x);
	}
	//Sets the left control
	void RCControls::setLeft(bool x)
	{
		setControl(1,x);
	}
	//Sets the backward control
	void RCControls::setBackward(bool x)
	{
		setControl(2,x);
	}
	//Sets the right control
	void RCControls::setRight(bool x)
	{
		setControl(3,x);
	}

//Actions---------------------------------------------------------------

	//Commands a picture to be taken
	void RCControls::takePicture()
	{
		setCommand(0, true);
	}

//Overriden Receiving Functions----------------------------------------

//Send remote data
int RCControls::sendRemote(uint8_t* msg, int len)
{
	int ret=2+BITTOBYTE((numCommand-1));

	//Confirm length
	if(len<ret)
	{
		cerr<<"The buffer cannot hold a remote message!"<<endl;
		return 0;
	}

	//Copy in single command recieve
	int cnt = 0;
	while(cnt<BITTOBYTE((numCommand-1)))
	{
		msg[cnt] = singleCommandSend[cnt];
		cnt++;
	}

	//Place in the power and steering app
	msg[cnt] = mainPower;
	msg[cnt+1] = turnAngle;

	return ret;
}
//Receive data as
void RCControls::receiveBase(uint8_t* msg, int len)
{
	int ret = 2+BITTOBYTE((numCommand-1));
	if(len != ret)
		return;

	int cnt = 0;
	//Run through all commands, flagging them if needed
	while(cnt<numCommand)
	{
		int ind = BITTOBYTE((cnt+1))-1;

		//A command has been received for the first time
		if(GETBIT(msg[ind],cnt-ind) && !(GETBIT(singleCommandRecieve[ind],cnt-ind)))
		{
			SETBIT(singleCommandRecieve[ind],cnt-ind,true);
			SETBIT(singleCommandSend[ind],cnt-ind,false);
		}
		//A command has been "depushed"
		else if(!GETBIT(msg[ind],cnt-ind))
		{
			SETBIT(singleCommandRecieve[ind],cnt-ind,false);
		}
		cnt++;
	}

	//Pull out power and angle
	mainPower = msg[cnt];
	turnAngle = msg[cnt+1];
}
//Reset the base data
void RCControls::resetBase()
{
	mainPower = 0;
	turnAngle = 0;
}

#endif