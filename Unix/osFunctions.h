//Primary author: Jonathan Bedard
//Confirmed working: 11/29/2014

//This is the file which contains the declarations for the OS unique functions

/*
MAC/LINUX
*/

#ifndef OSFUNCTIONS_H
#define OSFUNCTIONS_H

//Global macro functions
#define BITTOBYTE(x) ((x) > 0 ? (x/8+1) : 1)
#define BYTETOBIT(x) ((x) > 0 ? (x*8):0)
#define GETBIT(d,p) ((d) & (1 << (p)))
#define SETBIT(d,p,t) (t==0 ? d = (d)&~(1<<(p)):d = (d)|(1<<(p)))
#define IN_ADDR in_addr

#include <stdio.h>
#include <iostream>
#include <stdint.h>

#include <pthread.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "safeQueue.h"

#define BUFLEN 512  //Max length of buffer

using namespace std;

//These are global OS Functions

//Sleeps for x miliseconds
static void sleep(int x)
{
	usleep(x*1000);
}
//This function helps to start a thread, allows for simple unix interface
static void* temp_thread_call(void *ptr_array)
{
  void** break_array = (void**) ptr_array;
  void (*func) (void *);
  func = (void (*) (void *)) break_array[0];
  
  (*func)((void*) break_array[1]);
  
  delete(break_array);
}
//Spawns a thread
static void spawnThread(void (*func)(void *),void* ptr)
{
	void** ptr_array = new void*[2];
	ptr_array[0]  = (void*) func;
	ptr_array[1] = (void*) ptr;
	pthread_t hld;
	pthread_create(&hld,NULL,temp_thread_call,(void*)ptr_array);
}
//Tests if a folder exists and then creates it
static void testCreateFolder(std::string n)
{
	const char* path = n.c_str();
	mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
//Starts the IP library
static void startInternet()
{
	//No known start funcitons
}
//Closes the IP library
static void closeInternet()
{
	//No known clean-up functions
}

#define MY_MESSAGE_NOTIFICATION      1048

//This is the IP Address class
class IPAddress
{
protected:
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
	uint8_t* data;
	uint16_t length;
	uint8_t type;

public:
	//Initializers
	UDPPacket(uint8_t* input, IPAddress* i, int p);
	UDPPacket(uint8_t* output, int l, int t, IPAddress* i,int p);
	~UDPPacket();

	//Get Functions
	int getLength();
	int getType();
	uint8_t* getData();
	IPAddress* getAddress();
	int getPort();

	//Output Function
	uint8_t* sendData();
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
	socklen_t slen;

public:
	UDPServer(int port);
	~UDPServer();
	void start();
	void end();
	void recieveLoop();

	//Get/Set Functions
	bool getActive();
	bool getConnected();
	void setRecieveEvent(void (*func)(void *),void *ptr);

	//Send/Recieve
	bool send(UDPPacket* pck);
	bool available();
	UDPPacket* recieve();
};



#endif
