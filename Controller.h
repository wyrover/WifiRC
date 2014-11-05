//Primary author: Jonathan Bedard
//Confirmed working: 10/23/2014

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>

#include "CryptoGateway.h"
#include "Converter.h"
#include "serialThread.h"
#include "ConnectionManager.h"

//Controller Identifier
#define IDENTIFIER 1

//Controller macros
#define AUTO_SELECT 0
#define PREFFERED_AUTO 1
#define LIST_SELECTION 2
#define PREFFERED_LIST 3
#define ONE_SELECTION 4

#define DEFAULT_CHOICE 0
#define LIST_CHOICE 1
#define PREFERED_CHOICE 2
#define BLOCKED_CHOICE 3


#define AUTO_SELECT_STRING "Auto-Select"
#define PREFFERED_AUTO_STRING "Prefered_Auto"
#define LIST_SELECTION_STRING "List_Selection"
#define PREFFERED_LIST_STRING "Prefered_List"
#define ONE_SELECTION_STRING "One_Selection"

#define DEFAULT_CHOICE_STRING "Default"
#define LIST_CHOICE_STRING "List"
#define PREFERED_CHOICE_STRING "Prefered"
#define BLOCKED_CHOICE_STRING "Blocked"


#define AUTO_SELECT_UI "Auto-Select"
#define PREFFERED_AUTO_UI "Prefered Auto"
#define LIST_SELECTION_UI "List Selection"
#define PREFFERED_LIST_UI "Prefered List"
#define ONE_SELECTION_UI "One Selection"

#define DEFAULT_CHOICE_UI "Unselected"
#define LIST_CHOICE_UI "Selected"
#define PREFERED_CHOICE_UI "Primary"
#define BLOCKED_CHOICE_UI "Blocked"


using namespace std;

//The base controller declarations
class Controller:
	public receiveInterface
{
protected:
	char name_char[ID_SIZE];
	string name;
	string new_name;

	int intPort;
	RCControls *control;
	ConnectionManager* INetConnections;
	myIPAddress myAddress;

	int autoSave;
	bool validSave;
	int dicernType;

	bool blLAN;
	bool blPN;

	//Load/Save Functions
	string readTill(ifstream* file, char x);
	void loadError(ifstream* file);
	virtual void header_create(ofstream* saveFile);
	virtual void header_load(ifstream* loadFile);
	virtual void specific_save_data(ofstream* saveFile);
	virtual void specific_load_data(ifstream* loadFile);
	virtual string getSaveFileName();
public:
	Controller(void);
	virtual ~Controller(void);
	void late_initializations();

	//Set Functions
	void setName(string x);
	void setDicernType(int x);

	//Get Functions
	void printData();
	bool getFunctioning();
	int getDicernType();
	myIPAddress* getMyAddress();
	RCControls* getControls();
	ConnectionManager* getInetConnections();
	string getName();

	//Load/Save Functions
	void genSave();
	void genLoad();
};

//The Remote Controller declarations
class RemoteController :
	public Controller
{
private:
	bool timerCont;
	serialThread* serialCon;

	//Timer Functions
	void setControls();
	void sendData();

protected:
	//Load/Save Functions
	virtual void header_create(ofstream* saveFile);
	virtual void header_load(ifstream* loadFile);
	virtual string getSaveFileName();

public:
	RemoteController();
	virtual ~RemoteController();

	//Timer Functions
	void tick();
	virtual void receive_message(interior_message* msg);

	//Get Functions
	bool getFunctioning(); 
};

//The Base Controller declarations
class BaseController :
	public Controller
{
protected:
	bool timerCont;

	//Load/Save Functions
	virtual void header_create(ofstream* saveFile);
	virtual void header_load(ifstream* loadFile);
	virtual string getSaveFileName();
public:
	BaseController();
	virtual ~BaseController();

	//Timer Functions
	void tick();
	virtual void receive_message(interior_message* msg);
};

#endif