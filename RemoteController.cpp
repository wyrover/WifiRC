//Primary author: Jonathan Bedard
//Confirmed working: 10/27/2014

#ifndef REMOTECONTROLLER_CPP
#define REMOTECONTROLLER_CPP

#include "WifiRC.h"

#include <string>
#include <iostream>
#include <stdio.h>
#include <fstream>

#include "Controller.h"
#include "osFunctions.h"
#include "Converter.h"

#include "serialThread.h"

using namespace std;

//Static Functions

	//Static timer call
	static void remoteTimerTick(void* ptr)

	{
		sleep(1000);
		RemoteController * pthX = (RemoteController*)ptr;
		pthX->RemoteController::tick();
	}

//Constructors-------------------------------------------------------------------------------------

	//Constructor
	RemoteController::RemoteController(void):
		Controller()
	{
		INetConnections = new ConnectionManager((char*)"Files/Inet", name_char,&myAddress,intPort,0,1,&dicernType);

		timerCont = true;
		serialCon=new serialThread(false);

		late_initializations();

		spawnThread(&remoteTimerTick,this);
	}
	//Destructor
	RemoteController::~RemoteController(void)
	{
		delete(serialCon);
	}

//Timer Thread-------------------------------------------------------------------------------------

	//The main timer thread
	void RemoteController::tick()
	{
		interior_message msg;
		while(timerCont)
		{
			//Print out current data
			ID_module* mdl = INetConnections->getModule();
			if(mdl == NULL)
				cout<<"No Connection";
			else
				cout<<mdl->name_ID;
			INetConnections->returnModule();

			cout<<"\tPower: "<<((int) control->getPower());
			cout<<"\tAngle: "<<((int) control->getAngle());
			cout<<endl;

			//Sets the controls, base on the recieved input
			setControls();

			//Sends data to the arduino
			sendData();

			//Sends data through INetConnections
			if(INetConnections->isConnected()&&true)
			{
				//Sending a standard communication message
				msg.get_int_data()[4] = 1;
				int hld = control->sendRemote(&msg.get_int_data()[5],msg.get_full_length()-5);
				msg.push_length(5+hld);

				INetConnections->sendMessage(&msg);
			}

			//Check for the kill command
			if(!control->getOn())
				timerCont = false;
			sleep(100);
		}
		sleep(1000);
	}
	//Triggered anytime a message is received
	void RemoteController::receive_message(interior_message* msg)
	{
		if(msg==NULL)
			return;

		//Process a standard message
		if(msg->get_int_data()[4] == 1)
		{
			control->receiveRemote(&msg->get_int_data()[5],msg->get_length()-5);
			if(control->getPicture())
			{
				cout<<endl<<"------TAKING IMAGE-------"<<endl<<endl;
			}
		}
	}
	//Sends byte data to the serial connection
	void RemoteController::sendData()
	{
		static uint8_t arr[4];

		//Connection State LED
		ID_module* mdl = getInetConnections()->getModule();
		if(mdl != NULL && mdl->getConnectionStatus()>0)
			arr[0]=2;
		else
			arr[0]=1;
		getInetConnections()->returnModule();

		//Power byte
		arr[1] = (uint8_t) (control->getPower()+101);

		//Angle byte
		arr[2] = (uint8_t) (control->getAngle()+90);

		//Error Generation
		arr[3] = arr[0]^arr[1]^arr[2];

		if(serialCon!=NULL)
			serialCon->sendData(arr,4);
	}
	//Sets the controls
	void RemoteController::setControls()
	{
		//Power
		if(control->getForward())
			control->setPower(control->getPower()+5);
		if(control->getBackward())
			control->setPower(control->getPower()-5);

		//Angle
		if(control->getLeft())
			control->setAngle(control->getAngle()-5);
		if(control->getRight())
			control->setAngle(control->getAngle()+5);
	}

//Get Functions-----------------------------------------------------------------------------------

	//Returns true if the timer is ticking
	bool RemoteController::getFunctioning()
	{
		return timerCont;
	}

//Load/Save Functions-----------------------------------------------------------------------------

	//Creates the remote controller's header
	void RemoteController::header_create(ofstream* saveFile)
	{
		(*saveFile)<<"Remote_Controller_Info;"<<endl;
	}
	//Reads the remote controller's header
	void RemoteController::header_load(ifstream* loadFile)
	{
		string comp = readTill(loadFile,';');
		if(validSave&&comp!="Remote_Controller_Info")
		{
			cerr<<"Unexpected header"<<endl;
			loadError(loadFile);
		}
	}
	//Returns the save file name for the remote controller
	string RemoteController::getSaveFileName()
	{
		return "Remote_Controller";
	}

#endif

