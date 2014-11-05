//Primary author: Jonathan Bedard
//Confirmed working: 10/23/2014

#ifndef CONTROLLER_CPP
#define CONTROLLER_CPP

#include "WifiRC.h"
#include "osFunctions.h"
#include "ConnectionManager.h"

#include <string>
#include <sstream> 
#include <iostream>
#include <fstream>

using namespace std;

//Constructor-----------------------------------------------------------------

	//Default constructor
	Controller::Controller(void)
	{
		//Check ID first
		if(IDENTIFIER < 0 || IDENTIFIER > 31)
		{
			cerr<<"ID "<<IDENTIFIER<<" is out of the bounds of a good ID"<<endl;
			exit(EXIT_FAILURE);
		}

		//General Initializers
		intPort = 5252;
		dicernType = AUTO_SELECT;
		autoSave = 0;
		blLAN = true;
		blPN = true;
		name = "NULL";
		new_name = "";
		myAddress.getAddress();
		testCreateFolder("Files/");

		INetConnections = NULL;
		control = new RCControls();
		control->setOn(true);
	}
	//Default desructor
	Controller::~Controller(void)
	{
		if(INetConnections!=NULL)
			delete INetConnections;
		delete control;
	}
	//Triggers all of the late initializations
	void Controller::late_initializations()
	{
		if(INetConnections==NULL)
			cerr<<"The connections should have been established! Cannot bind receiver!"<<endl;
		else
			INetConnections->setReceiver(this);
		genLoad();
		genSave();
	}

//Set Functions--------------------------------------------------------------

	//Sets the name of the function
	void Controller::setName(string x)
	{
		new_name = x;
	}
	//Sets the dicern type
	void Controller::setDicernType(int x)
	{
		dicernType = x;
		genSave();
	}

//Get Functions--------------------------------------------------------------

	//Prints out the basic controller data
	void Controller::printData()
	{
		cout<<"Data"<<endl;
		cout<<"Port:"<<intPort<<endl;
		cout<<"Name:"<<name<<endl;
		cout<<"LAN:"<<blLAN<<endl;
		cout<<"PN:"<<blPN<<endl;
	}
	//Returns if the controller is still functioning
	bool Controller::getFunctioning()
	{
		return true;
	}
	//Returns the dicernment type
	int Controller::getDicernType()
	{
		return dicernType;
	}
	//Returns a pointer to the IP address data structure
	myIPAddress* Controller::getMyAddress()
	{
		return &myAddress;
	}
	//Return a pointer to the RCControls structure
	RCControls* Controller::getControls()
	{
		return control;
	}
	//Return a pointer to the Connection manager
	ConnectionManager* Controller::getInetConnections()
	{
		return INetConnections;
	}
	//Return the name of the controller
	string Controller::getName()
	{
		return name;
	}

//Load/Save Functions-------------------------------------------------------

	//Saves the controller
	void Controller::genSave()
	{
		ofstream saveFile;
		saveFile.open (("Files/"+getSaveFileName()+".rcf").c_str());

		//Create header
		header_create(&saveFile);

		if(new_name=="")
			saveFile<<">name:"<<name<<";"<<endl;
		else
			saveFile<<">name:"<<new_name<<";"<<endl;
		saveFile<<">port:"<<intPort<<";"<<endl;

		//Connection Type
		saveFile<<">type:";
		if(dicernType==PREFFERED_AUTO)
			saveFile<<PREFFERED_AUTO_STRING;
		else if(dicernType==LIST_SELECTION)
			saveFile<<LIST_SELECTION_STRING;
		else if(dicernType==PREFFERED_LIST)
			saveFile<<PREFFERED_LIST_STRING;
		else if(dicernType==ONE_SELECTION)
			saveFile<<ONE_SELECTION_STRING;
		else
		{
			saveFile<<AUTO_SELECT_STRING;
			dicernType=AUTO_SELECT;
		}

		saveFile<<";"<<endl;

		//LAN Flag
		saveFile<<">LAN:";
		if(blLAN)
			saveFile<<"YES";
		else
			saveFile<<"NO";

		//Public Network Flag
		saveFile<<";"<<endl<<">PN:";
		if(blPN)
			saveFile<<"YES";
		else
			saveFile<<"NO";
		saveFile<<";"<<endl;

		//Specific Save data
		specific_save_data(&saveFile);

		//Close
		saveFile<<">end:;";
		saveFile.close();
	}
	//Loads the controller
	void Controller::genLoad()
	{
		validSave = true;
		ifstream loadFile;
		loadFile.open (("Files/"+getSaveFileName()+".rcf").c_str());

		//Check header
		if(validSave)
			header_load(&loadFile);

		//Load Name
		string hold;
		if(validSave)
		{
			readTill(&loadFile, '>');
			hold=readTill(&loadFile, ':');
			if(validSave&&hold=="name")
			{
				hold = readTill(&loadFile,';');
				if(validSave)
				{
					if(hold.length()>ID_SIZE)
						name = hold.substr(0,ID_SIZE);
					else
						name = hold;

					int cnt = 0;
					while(cnt<ID_SIZE)
					{
						if(cnt<name.length())
							name_char[cnt] = name.at(cnt);
						else
							name_char[cnt] = '\0';
						cnt++;
					}
				}
			}
			else
			{
				cerr<<"Name line corrupted"<<endl;
				loadError(&loadFile);
			}
		}

		//Load Port
		if(validSave)
		{
			readTill(&loadFile, '>');
			hold=readTill(&loadFile, ':');
			if(validSave&&hold=="port")
			{
				hold = readTill(&loadFile,';');
				if(validSave)
				{
					stringstream convert(hold);
					int temp;
					if((convert >> temp))
						intPort = temp;
					else
					{
						cerr<<"Invalid port indentifier"<<endl;
						loadError(&loadFile);
					}
				}
			}
			else
			{
				cerr<<"Port line corrupted"<<endl;
				loadError(&loadFile);
			}
		}

		//Load Dicern Type
		if(validSave)
		{
			readTill(&loadFile, '>');
			hold=readTill(&loadFile, ':');
			if(validSave&&hold=="type")
			{
				hold = readTill(&loadFile,';');
				if(validSave)
				{
					if(hold==PREFFERED_AUTO_STRING)
						dicernType = PREFFERED_AUTO;
					else if(hold==LIST_SELECTION_STRING)
						dicernType=LIST_SELECTION;
					else if(hold==PREFFERED_LIST_STRING)
						dicernType=PREFFERED_LIST;
					else if(hold==ONE_SELECTION_STRING)
						dicernType=ONE_SELECTION;
					else
						dicernType = AUTO_SELECT;
				}
			}
			else
			{
				cerr<<"Type line corrupted"<<endl;
				loadError(&loadFile);
			}
		}

		//LAN Flag
		if(validSave)
		{
			readTill(&loadFile, '>');
			hold=readTill(&loadFile, ':');
			if(validSave&&hold=="LAN")
			{
				hold = readTill(&loadFile,';');
				if(validSave)
				{
					if(hold == "YES")
						blLAN = true;
					else
						blLAN = false;
				}
			}
			else
			{
				cerr<<"LAN line corrupted"<<endl;
				loadError(&loadFile);
			}
		}

		//Private Network Flag
		if(validSave)
		{
			readTill(&loadFile, '>');
			hold=readTill(&loadFile, ':');
			if(validSave&&hold=="PN")
			{
				hold = readTill(&loadFile,';');
				if(validSave)
				{
					if(hold == "YES")
						blPN = true;
					else
						blPN = false;
				}
			}
			else
			{
				cerr<<"PN line corrupted"<<endl;
				loadError(&loadFile);
			}
		}

		//Specific Load
		if(validSave)
			specific_load_data(&loadFile);

		if(validSave)
			loadFile.close();
	}
	//Reads till the specified character
	string Controller::readTill(ifstream* file, char x)
	{
		if(!validSave)
			return "";
		string ret = "";
		char t = x+1;

		while((*file).good()&&t!=x)
		{
			(*file)>>t;
			if(x!=t)
				ret = ret+t;
		}

		//Trigger load error
		if(!file->good())
		{
			cerr<<"File out of bounds"<<endl;
			loadError(file);
			ret = "";
		}
		return ret;
	}
	//Triggers during a load error
	void Controller::loadError(ifstream* file)
	{
		validSave = false;
		file->close();
		cerr<<"Emergency save"<<endl;
		genSave();
	}
	//Creates the header
	void Controller::header_create(ofstream* saveFile)
	{
		(*saveFile)<<"Void_Controller_Info;"<<endl;
	}
	//Loads the header
	void Controller::header_load(ifstream* loadFile)
	{
		string comp = readTill(loadFile,';');
		if(validSave&&comp!="Void_Controller_Info")
		{
			cerr<<"Unexpected header"<<endl;
			loadError(loadFile);
		}
	}
	//Saves the data specific to the controller type
	void Controller::specific_save_data(ofstream* saveFile)
	{
		(*saveFile)<<">SPEC:VOID;"<<endl;
	}
	//Loads the data specific to the controller type
	void Controller::specific_load_data(ifstream* loadFile)
	{
		readTill(loadFile, '>');
		string comp = readTill(loadFile,';');
		if(validSave&&comp!="SPEC:VOID")
		{
			cerr<<"Void load data corrupted"<<endl;
			loadError(loadFile);
		}
	}
	//Returns the name of the save file the controller is using
	string Controller::getSaveFileName()
	{
		return "VOID_controller";
	}

#endif
