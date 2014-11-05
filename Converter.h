//Primary author: Jonathan Bedard
//Confirmed working: 10/27/2014

#ifndef CONVERTER_H
#define CONVERTER_H

#include <stdint.h>

//The General Controls
class GenControls
{
protected:	
	uint8_t* formControls;
	uint8_t* singleCommandSend;
	uint8_t* singleCommandRecieve;
	int numControls;
	int numCommand;
public:
	//Constructors/Destructors
	GenControls(int numCon, int numCom);
	virtual ~GenControls(void);

	//Reset
	void resetControls();
	void resetCommandSend();
	void resetCommandRecieve();

	//Get Functions
	int getNumCommands();
	int getNumControls();
	bool getCommand(int x);
	bool getControl(int x);

	//Set Functions
	void setCommand(int x,bool d);
	void setControl(int x,bool d);

	//Sending
	virtual int sendRemote(uint8_t* msg, int len);
	virtual int sendBase(uint8_t* msg, int len);

	//Receiving
	virtual void receiveRemote(uint8_t* msg, int len);
	virtual void receiveBase(uint8_t* msg, int len);
	virtual void resetBase();
	virtual void resetRemote();

};

//The RCControls class
class RCControls:
	public GenControls
{
private:
	int8_t powerBound;
	int8_t mainPower;
	int8_t boundAngle;
	int8_t turnAngle;
public:
	//Constructors/Destructors
	RCControls(void);
	virtual ~RCControls(void);

	//Get Functions
	int8_t getPower();
	int8_t getAngle();
	bool getOn();
	bool getForward();
	bool getLeft();
	bool getBackward();
	bool getRight();

	//Get Commands
	bool getPicture();

	//Set Functions
	void setPower(int x);
	void setAngle(int x);
	void setOn(bool x);
	void setForward(bool x);
	void setLeft(bool x);
	void setBackward(bool x);
	void setRight(bool x);

	//Action Funcitons
	void takePicture();

	//Sending
	virtual int sendRemote(uint8_t* msg, int len);

	//Receiving
	virtual void receiveBase(uint8_t* msg, int len);
	virtual void resetBase();
};

#endif