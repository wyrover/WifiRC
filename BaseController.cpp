//Primary author: Jonathan Bedard
//Confirmed working: 10/23//2014

#ifndef BASECONTROLLER_CPP
#define BASECONTROLLER_CPP

#include <string>
#include <iostream>
#include <fstream>

#include "WifiRC.h"
#include "Controller.h"

//Static Functions

	//Static timer call
	static void baseTimerTick(void* ptr)
	{
		sleep(1000);
		BaseController * pthX = (BaseController*)ptr;
		pthX->BaseController::tick();
	}

//Initializers----------------------------------------------------------------------------------------------------------------------------

	//Constructor
	BaseController::BaseController(void)
	{
		INetConnections = new ConnectionManager((char*)"Files/Inet",name_char,&myAddress,intPort,1,1,&dicernType);
		late_initializations();
		timerCont = true;
		
		spawnThread(&baseTimerTick,this);
	}
	//Destructor
	BaseController::~BaseController(void)
	{
		timerCont = false;
		sleep(200);
	}

//Timer Functions-------------------------------------------------------------------------------------------------------------------------

	//Triggers every time a message needs to be sent
	void BaseController::tick()
	{
		interior_message msg;
		//Continues until the delete
		while(timerCont)
		{
			//Sends data through INetConnections
			if(INetConnections->isConnected())
			{
				//Sending a standard communication message
				
				msg.get_int_data()[4] = 1;
				int hld = control->sendBase(&msg.get_int_data()[5],msg.get_full_length()-5);
				msg.push_length(5+hld);

				INetConnections->sendMessage(&msg);
			}

			if(timerCont)
				sleep(100);
		}
	}
	//Triggered anytime a message is received
	void BaseController::receive_message(interior_message* msg)
	{
		if(msg==NULL)
			return;

		//Process a standard message
		if(msg->get_int_data()[4] == 1)
		{
			control->receiveBase(&msg->get_int_data()[5],msg->get_length()-5);
		}
	}

//Get Functions---------------------------------------------------------------------------------------------------------------------------


//Load/Save Functions---------------------------------------------------------------------------------------------------------------------

	//Creates the remote controller's header
	void BaseController::header_create(ofstream* saveFile)
	{
		(*saveFile)<<"Base_Controller_Info;"<<endl;
	}
	//Reads the remote controller's header
	void BaseController::header_load(ifstream* loadFile)
	{
		string comp = readTill(loadFile,';');
		if(validSave&&comp!="Base_Controller_Info")
		{
			cerr<<"Unexpected header"<<endl;
			loadError(loadFile);
		}
	}
	//Returns the save file name for the remote controller
	string BaseController::getSaveFileName()
	{
		return "Base_Controller";
	}
#endif