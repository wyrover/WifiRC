//Primary author: Jonathan Bedard
//Confirmed working: 2/13/2015

//This is the file which contains the declarations for the OS unique functions

/*
WINDOWS ONLY
*/

#ifndef OSFUNCTIONS_H
#define OSFUNCTIONS_H

//Global macro functions
#define BITTOBYTE(x) ((x) > 0 ? (x/8+1) : 1)
#define BYTETOBIT(x) ((x) > 0 ? (x*8):0)
#define GETBIT(d,p) ((d) & (1 << (p)))
#define SETBIT(d,p,t) (t==0 ? d = (d)&~(1<<(p)):d = (d)|(1<<(p))) 

#include <stdio.h>
#include <iostream>
#include <stdint.h>

#include <process.h>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include <iphlpapi.h>
#include <time.h>

#include "safeQueue.h"

#define BUFLEN 512  //Max length of buffer

using namespace std;

//These are global OS Functions

//Sleeps for x miliseconds
static void sleep(int x)
{
	Sleep(x);
}
//Spawns a thread
static void spawnThread(void (*func)(void *),void *ptr)
{
	_beginthread(*func,0,ptr);
}
//Converts char* to wchar_t
static wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString=new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}
//Changes an int to compatibility mode
static uint16_t to_comp_mode(uint16_t i)
{
	uint16_t temp = 1;
	//Switch little to big endian
	if(((char*) &temp)[0] == 0)
	{
		((char*) &temp)[0] = ((char*) &i)[1];
		((char*) &temp)[1] = ((char*) &i)[0];
		return temp;
	}
	return i;
}
//Changes an int from compatibility mode to system mode
static uint16_t from_comp_mode(uint16_t i)
{
	uint16_t temp = 1;
	//Switch little to big endian
	if(((char*) &temp)[0] == 0)
	{
		((char*) &temp)[0] = ((char*) &i)[1];
		((char*) &temp)[1] = ((char*) &i)[0];
		return temp;
	}
	return i;
}
//Changes an int to compatibility mode
static uint32_t to_comp_mode(uint32_t i)
{
	uint32_t temp = 1;
	//Switch little to big endian
	if(((char*) &temp)[0] == 0)
	{
		((char*) &temp)[0] = ((char*) &i)[3];
		((char*) &temp)[1] = ((char*) &i)[2];
		((char*) &temp)[2] = ((char*) &i)[1];
		((char*) &temp)[3] = ((char*) &i)[0];
		return temp;
	}
	return i;
}
//Changes an int from compatibility mode to system mode
static uint32_t from_comp_mode(uint32_t i)
{
	uint32_t temp = 1;
	//Switch little to big endian
	if(((char*) &temp)[0] == 0)
	{
		((char*) &temp)[0] = ((char*) &i)[3];
		((char*) &temp)[1] = ((char*) &i)[2];
		((char*) &temp)[2] = ((char*) &i)[1];
		((char*) &temp)[3] = ((char*) &i)[0];
		return temp;
	}
	return i;
}
//Tests if a folder exists and then creates it
static void testCreateFolder(std::string n)
{
	const char* path = n.c_str();
	CreateDirectory(path ,NULL);
}
//Starts the IP library
static void startInternet()
{
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0)
	{
		cerr<<"Invalid initialization of the WSA dll"<<endl;
		exit(EXIT_FAILURE);
	}
}
//Closes the IP library
static void closeInternet()
{
	WSACleanup();
}

#define MY_MESSAGE_NOTIFICATION      1048

//This is the IP Address class
class IPAddress
{
private:
	char name[80];

public:
	IPAddress();
	IPAddress(string x);
	IPAddress(IN_ADDR x);
	IPAddress(IPAddress* x);
	~IPAddress();

	IN_ADDR address;

	char* printAddress();
	int compare(const IPAddress* comp) const;
};

//This is the class which holds my IP address
class myIPAddress
{
private:
	IN_ADDR address;
	IN_ADDR resetAddress();
	clock_t last;
	char hostName[80];
	char name[80];

public:
	myIPAddress();
	~myIPAddress();
	IPAddress getAddress();
	char* getIPString();
	char* getHostName();
};

//This is the UDP Packet class, that can be sent and received
class UDPPacket
{
private:
	IPAddress ip;
	bool in_or_out;
	int port;
	byte* data;
	uint16_t length;
	uint8_t type;

public:
	//Initializers
	UDPPacket(byte* input, IPAddress* i, int p);
	UDPPacket(byte* output, int l, int t, IPAddress* i,int p);
	~UDPPacket();

	//Get Functions
	int getLength();
	int getType();
	byte* getData();
	IPAddress* getAddress();
	int getPort();

	//Output Function
	byte* sendData();
};

//These are the UDP client and server class, respectively

//This is the UDP Client
class UDPClient
{
private:
	int intPort;
	bool active;
	volatile bool connected;
	int conTrack;
	int resetVal;

	void (*recieveFunction)(void *);
	void *recievePointer;

	safeQueue<UDPPacket> incomingPackets;
	spinLock safeDelete;
	spinLock popLock;

	struct sockaddr_in si_other;
    int s;
	int slen;

public:
	UDPClient(int port, IPAddress address);
	~UDPClient();
	void connect();
	void disconnect();
	void recieveLoop();

	//Get Functions
	bool getActive();
	bool getConnected();

	//Set Functions
	void setReset(int x);
	void setRecieveEvent(void (*func)(void *),void *ptr);

	//Timer Functions
	void tick();

	//Send/Recieve
	bool send(UDPPacket* pck);
	bool available();
	UDPPacket* recieve();
};

//This is the UDP Server
class UDPServer
{
private:
	int intPort;
	bool active;
	volatile bool connected;

	void (*recieveFunction)(void *);
	void *recievePointer;

	safeQueue<UDPPacket> incomingPackets;
	spinLock safeDelete;
	spinLock popLock;

	struct sockaddr_in si_other;
	struct sockaddr_in server;
    int s;
	int slen;

public:
	UDPServer(int port);
	~UDPServer();
	void start();
	void end();
	void recieveLoop();

	//Get/Set Functions
	bool getActive();
	void setRecieveEvent(void (*func)(void *),void *ptr);

	//Send/Recieve
	bool send(UDPPacket* pck);
	bool available();
	bool getConnected();
	UDPPacket* recieve();
};



#endif