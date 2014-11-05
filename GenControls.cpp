//Primary author: Jonathan Bedard
//Confirmed working: 10/27/2014

#ifndef GENCONTROLS_CPP
#define GENCONTROLS_CPP

#include "WifiRC.h"
#include <stdint.h>
#include <string>
#include <iostream>
#include <stdio.h>

#include "Converter.h"
#include "osFunctions.h"

using namespace std;



//Constructor-------------------------------------------------------------
	//Constructor
	GenControls::GenControls(int numCon, int numCom)
	{
		if(numCon<0||numCom<0)
		{
			cerr<<"Invalid parameter to General Controls.  Abort."<<endl;
			exit(0);
		}

		numControls=numCon;
		numCommand=numCom;

		formControls=new uint8_t[BITTOBYTE((numControls-1))];
		singleCommandSend=new uint8_t[BITTOBYTE((numCommand-1))];
		singleCommandRecieve=new uint8_t[BITTOBYTE((numCommand-1))];

		resetControls();
		resetCommandSend();
		resetCommandRecieve();
	}
	//Destructor
	GenControls::~GenControls()
	{
		delete[] formControls;
		delete[] singleCommandSend;
		delete[] singleCommandRecieve;
	}

//Reset Functions---------------------------------------------------------
	//Reset Controls
	void GenControls::resetControls()
	{
		int cnt = BITTOBYTE(numControls);
		while (cnt>0)
		{
			cnt--;
			formControls[cnt]=0;
		}
	}
	//Reset sent commands
	void GenControls::resetCommandSend()
	{
		int cnt = BITTOBYTE(numCommand);
		while (cnt>0)
		{
			cnt--;
			singleCommandSend[cnt]=0;
		}
	}
	//Reset recieved commands
	void GenControls::resetCommandRecieve()
	{
		int cnt = BITTOBYTE(numCommand);
		while (cnt>0)
		{
			cnt--;
			singleCommandRecieve[cnt]=0;
		}
	}

//Get Functions-----------------------------------------------------------
	//Returns the number of command bits
	int GenControls::getNumCommands()
	{
		return numCommand;
	}
	//Returns the number of control bits
	int GenControls::getNumControls()
	{
		return numControls;
	}
	//Returns the state of a command bit
	bool GenControls::getCommand(int x)
	{
		if(x<0&&x>=numCommand)
		{
			cerr<<"Invalid command arguement.  Abort"<<endl;
			exit(0);
		}

		int ind = BITTOBYTE((x+1))-1;
		
		return (bool) GETBIT(singleCommandRecieve[ind],x-ind);
	}
	//Returns the state of a control bit
	bool GenControls::getControl(int x)
	{
		if(x<0&&x>=numControls)
		{
			cerr<<"Invalid control arguement.  Abort"<<endl;
			exit(0);
		}

		int ind = BITTOBYTE((x+1))-1;
		return (bool) GETBIT(formControls[ind],x-ind);
	}

//Set Functions-----------------------------------------------------------

	//Sets an individual command
	void GenControls::setCommand(int x,bool d)
	{
		if(x<0&&x>=numCommand)
		{
			cerr<<"Invalid command arguement.  Abort"<<endl;
			exit(0);
		}

		int ind = BITTOBYTE((x+1))-1;

		if(d==false || (!GETBIT(singleCommandSend[ind],x-ind) && !GETBIT(singleCommandRecieve[ind],x-ind)))
			SETBIT(singleCommandSend[ind],x-ind,d);
	}
	//Sets an individual control
	void GenControls::setControl(int x,bool d)
	{
		if(x<0&&x>=numControls)
		{
			cerr<<"Invalid control arguement.  Abort"<<endl;
			exit(0);
		}

		int ind = BITTOBYTE((x+1))-1;
		SETBIT(formControls[ind],x-ind,d);
	}

//Sending-----------------------------------------------------------------
	
	//Builds the remote message, return the length of the message
	int GenControls::sendRemote(uint8_t* msg, int len)
	{
		int ret=BITTOBYTE((numCommand-1));

		//Confirm length
		if(len<ret)
		{
			cerr<<"The buffer cannot hold a remote message!"<<endl;
			return 0;
		}

		//Copy in single command send
		int cnt = 0;
		while(cnt<ret)
		{
			msg[cnt] = singleCommandSend[cnt];
			cnt++;
		}

		return ret;
	}
	//Builds the base message, returns the length of the message
	int GenControls::sendBase(uint8_t* msg, int len)
	{
		int ret=BITTOBYTE((numCommand-1))+BITTOBYTE((numControls-1));

		//Confirm length
		if(len<ret)
		{
			cerr<<"The buffer cannot hold a remote message!"<<endl;
			return 0;
		}

		//Copy in single command send
		int cnt = 0;
		while(cnt<BITTOBYTE((numCommand-1)))
		{
			msg[cnt] = singleCommandSend[cnt];
			cnt++;
		}
		//Copy in controls
		while(cnt<ret)
		{
			msg[cnt] = formControls[cnt-BITTOBYTE((numCommand-1))];
			cnt++;
		}

		return ret;
	}

//Receiving---------------------------------------------------------------
	
	//Receive a message from the base
	void GenControls::receiveRemote(uint8_t* msg, int len)
	{
		//Confirm the length of the message
		int ret = BITTOBYTE((numCommand-1))+BITTOBYTE((numControls-1));
		if(len != ret)
			return;

		//Process
		
		//Process commands
		int cnt = 0;

		//Set all sent command bits to 0
		while(cnt<BITTOBYTE((numCommand-1)))
		{
			singleCommandRecieve[cnt] = 0;
			cnt++;
		}

		cnt = 0;
		//Run through all commands, flagging them if needed
		while(cnt<numCommand)
		{
			int ind = BITTOBYTE((cnt+1))-1;

			//A command has been received for the first time
			if((GETBIT(msg[ind],(cnt-ind))) && (!(GETBIT(singleCommandSend[ind],(cnt-ind)))))
			{
				SETBIT(singleCommandRecieve[ind],cnt-ind,true);
				SETBIT(singleCommandSend[ind],cnt-ind,true);
			}
			//A command has been "depushed"
			else if(!(GETBIT(msg[ind],(cnt-ind))))
			{
				SETBIT(singleCommandRecieve[ind],cnt-ind,false);
				SETBIT(singleCommandSend[ind],cnt-ind,false);
			}
			else
				SETBIT(singleCommandRecieve[ind],cnt-ind,false);
			cnt++;
		}

		//Copy out controls
		cnt = BITTOBYTE((numCommand-1));
		while(cnt<ret)
		{
			formControls[cnt-BITTOBYTE((numCommand-1))] = msg[cnt];
			cnt++;
		}
	}
	//Receive a message from the remote
	void GenControls::receiveBase(uint8_t* msg, int len)
	{
		int ret = BITTOBYTE((numCommand-1));
		if(len != ret)
			return;

		int cnt = 0;
		//Run through all commands, flagging them if needed
		while(cnt<numCommand)
		{
			int ind = BITTOBYTE((cnt+1))-1;

			//A command has been received for the first time
			if(GETBIT(msg[ind],cnt-ind) && ! (GETBIT(singleCommandRecieve[ind],cnt-ind)))
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
	}
	//Sets all commands and controls to zero
	void GenControls::resetRemote()
	{
		int cnt = 0;
		while(cnt<numControls)
		{
			formControls[cnt] = 0;
			cnt++;
		}
		while(cnt<numCommand)
		{
			singleCommandSend[cnt] = 0;
			singleCommandRecieve[cnt] = 0;
			cnt++;
		}
	}
	//No action required
	void GenControls::resetBase()
	{
		//No actin required
	}

#endif