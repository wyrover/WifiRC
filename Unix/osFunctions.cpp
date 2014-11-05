//Primary author: Jonathan Bedard
//Confirmed working: 11/2/2014

/*
MAC/LINUX
*/

#ifndef OSFUNCTIONS_CPP
#define OSFUNCTIONS_CPP

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>

#include <sys/types.h>
#include <ifaddrs.h>

#include "Serial.h"
#include "safeQueue.h"
#include "osFunctions.h"

//All other operating systems must have their own .cpp file created

//Serial Communication-------------------------------------------------------------------------------------------------------------------
	//Initializes the serial port
	Serial::Serial(char *portName, bool t)
	{
	    struct termios options;
	  
	    track = t;
	    //We're not yet connected
	    connected = false;

	    //Try to connect to the given port throuh CreateFile
	    hSerial = open(portName,O_RDWR | O_NOCTTY | O_NDELAY);
	    //Check if the connection was successfull
	    if(hSerial==-1)
	    {
		if(track)
			cout<<"Connection to "<<portName<<" failed"<<endl;
		sleep(10);
	    }
	    else
	    {
	        //Try to get the current
		if (tcgetattr(hSerial, &options)==-1)
		{
			//If impossible, show an error
			if(track)
				cout<<"failed to get current serial parameters "<<" on "<<portName<<endl;
			else
				sleep(10);
		}
		else
		{
		    cfsetispeed(&options, B9600);
		    cfsetospeed(&options, B9600);
		    options.c_cflag |= (CLOCAL | CREAD);
		    options.c_cflag &= ~PARENB;
		    options.c_cflag &= ~CSTOPB;
		    options.c_cflag &= ~CSIZE;
		    options.c_cflag |= CS8;	
		    
		    //Set the parameters and check for their proper application
		    if(tcsetattr(hSerial, TCSANOW, &options)==-1)
		    {
			if(track)
			  cout<<"Could not set Serial Port parameters on "<<portName<<endl;
			else
			  sleep(10);
		    }
		    else
		    {
			//If everything went fine we're connected
			connected = true;
			//We wait 2s as the arduino board will be reseting
			sleep(ARDUINO_WAIT_TIME);
			if(track)
			  cout<<"Successful Connection on "<<portName<<endl;
		    }
		}
	    }
	}
	//Destroys the serial port
	Serial::~Serial()
	{
	    //Check if we are connected before trying to disconnect
	    if(connected)
	    {
	        //We're no longer connected
	        connected = false;
	        //Close the serial handler
	        close(hSerial);
	    }
	}
	//Reads data from the serial port
	int Serial::ReadData(uint8_t *buffer, unsigned int nbChar)
	{
	    usleep(nbChar*100);
	    //Number of bytes read
	    int toRead = read(hSerial,buffer,nbChar);

	    //Check if there is something to read
	    if(toRead > 0)
		return toRead;
	    //If nothing has been read, or that an error was detected return -1
	    if(track)
		cerr<<"Error reading from the Com port"<<endl;
	    return -1;
	}
	//Writes data to the serial port
	bool Serial::WriteData(uint8_t *buffer, unsigned int nbChar)
	{
	    //Try to write the buffer on the Serial port
	    if(write(hSerial,buffer,nbChar)==-1)
	    {
		usleep(nbChar*100);
		if(track)
		    cerr<<"Com error in transmission"<<endl;
	        return false;
	    }
	    else
	    {
		usleep(nbChar*100);
		return true; 
	    }
	}
	//Tests if the serial port is connected
	bool Serial::IsConnected()
	{
		//Simply return the connection status
		return this->connected;
	}

//IPAddress Class------------------------------------------------------------------------------------------------------------------------

	//Constructor
	IPAddress::IPAddress()
	{
		inet_pton(AF_INET, "127.0.0.1", &(address));
		inet_ntop(AF_INET,&address,name,80);
	}
	IPAddress::IPAddress(string x)
	{
		inet_pton(AF_INET, x.c_str(), &(address));
		inet_ntop(AF_INET,&address,name,80);
	}
	IPAddress::IPAddress(sockaddr_in x)
	{
		address = x;
		inet_ntop(AF_INET,&address,name,80);
	}
	IPAddress::IPAddress(IPAddress* x)
	{
		address = x->address;
		inet_ntop(AF_INET,&address,name,80);
	}
	//Destructor
	IPAddress::~IPAddress()
	{
		//No clean up needed
	}
	//Returns the string of an IP address
	char* IPAddress::printAddress()
	{
		inet_ntop(AF_INET,&address,name,80);
		return name;
	}
	//Compares to IP addresses for equality
	int IPAddress::compare(const IPAddress* comp) const
	{
		uint32_t* me = (uint32_t*) &(address.sin_addr);
		uint32_t* them = (uint32_t*) &(comp->address.sin_addr);
		if((*me)==(*them))
			return 0;
		if((*me)>(*them))
			return 1;
		return -1;
	}

//Constructor/Destructor-----------------------------------------------------------------------------------------------------------------

	//Constructor
	myIPAddress::myIPAddress()
	{
		address = resetAddress();
		inet_ntop(AF_INET,&address,name,80);
	}
	//Destructor
	myIPAddress::~myIPAddress()
	{
	}
	//Resets the address
	sockaddr_in myIPAddress::resetAddress()
	{
		last = clock();
		sockaddr_in ret;
		inet_pton(AF_INET, "127.0.0.1", &(ret));
		
		ifaddrs* addrs;
		ifaddrs* ip_array;
		getifaddrs(&addrs);
		ip_array = addrs;
		
		while(ip_array)
		{
		  if (ip_array->ifa_addr && ip_array->ifa_addr->sa_family == AF_INET)
		  {
		      struct sockaddr_in *pAddr = (struct sockaddr_in *)ip_array->ifa_addr;
		      inet_pton(AF_INET, inet_ntoa(pAddr->sin_addr), &(ret));
		  }
		  ip_array = ip_array->ifa_next;
		}
		
		freeifaddrs(addrs);

		return ret;
	}

//Get Functions--------------------------------------------------------------------------------------------------------------------------

	//Returns the address
	IPAddress myIPAddress::getAddress()
	{
		if((float)(clock()-last)/CLOCKS_PER_SEC>2)
			address = resetAddress();
		return IPAddress(address);
	}
	//Returns the string of the IP address
	char* myIPAddress::getIPString()
	{
		inet_ntop(AF_INET,&address,name,80);
		return name;
	}
	//Returns the host name of this computer
	char* myIPAddress::getHostName()
	{
		return hostName;
	}

//Data Packet Class----------------------------------------------------------------------------------------------------------------------

	//Receiving initializer
	UDPPacket::UDPPacket(uint8_t* input, IPAddress* i, int p)
	{
		ip.address = i->address;

		//Unpack the type
		type = (uint8_t) input[0]>>1;
		type = type & 127;

		//Unpack the length
		length = (uint16_t) input[1];
		length = (uint16_t) length | ((input[0]&1)<<8);

		//Unpack the data
		in_or_out = true;
		data = new uint8_t[length];
		int cnt = 0;

		while(cnt<length)
		{
			data[cnt]= input[cnt+2];
			cnt++;
		}
		port = p;
	}
	//Sending initializer
	UDPPacket::UDPPacket(uint8_t* output, int l, int t, IPAddress* i,int p)
	{
		ip.address = i->address;

		//Length
		if(l>0 && l<510)
			length = (uint16_t) l;
		else
		{
			cerr<<"Invalid packet length.  Abort."<<endl;
			exit(EXIT_FAILURE);
		}

		//Type
		if(t>=0 && t<128)
			type = (uint8_t) t;
		else
		{
			cerr<<"Invalid packet length.  Abort."<<endl;
			exit(EXIT_FAILURE);
		}
		in_or_out = false;
		data = output;
		port = p;
	}
	//Destructor
	UDPPacket::~UDPPacket()
	{
		if(in_or_out)
			delete [] data;
	}

//Get Functions--------------------------------------------------------------------------------------------------------------------------
	
	//Returns the length of the packet
	int UDPPacket::getLength()
	{
		return length;
	}
	//Returns the type of the packet
	int UDPPacket::getType()
	{
		return type;
	}
	//Returns the data in the packet
	uint8_t* UDPPacket::getData()
	{
		return data;
	}
	//Returns the address attached to the packet
	IPAddress* UDPPacket::getAddress()
	{
		return &ip;
	}
	//Returns the port the value came from/is headed to
	int UDPPacket::getPort()
	{
		return port;
	}

	//Output Function
	uint8_t* UDPPacket::sendData()
	{
		uint8_t* ret = new uint8_t[length+2];
		int cnt = 0;

		//Transfer the data
		while(cnt<length)
		{
			ret[cnt+2] = data[cnt];
			cnt++;
		}

		//Pack the type and length
		ret[1] = (uint8_t) length;
		ret[0] = (uint8_t) (length>>8);
		ret[0] = (uint8_t) ret[0] | (type<<1);

		return ret;
	}

//UDP Client-----------------------------------------------------------------------------------------------------------------------------

	//The static function which begins the recieve loop
	static void recieveThreadClient(void* ptr)
	{
		UDPClient * pthX = (UDPClient*)ptr;
		pthX->UDPClient::recieveLoop();
	}
	//This is the static timer functions
	static void UDPTimer(void* ptr)
	{
		UDPClient * pthX = (UDPClient*)ptr;
		pthX->UDPClient::tick();
	}

//Constructor/Destructor-----------------------------------------------------------------------------------------------------------------

	//Constructor
	UDPClient::UDPClient(int port, IPAddress address)
	{
		slen=sizeof(si_other);
		active = true;
		connected = false;
		intPort = port;

		conTrack = 0;
		resetVal = 1500;

		recieveFunction = NULL;
		recievePointer = NULL;

		//create socket
		if ( active&&(s=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			cerr<<"Socket failed to open"<<endl;
			active = false;
		}
 
		if(active)
		{
			//setup address structure
			memset(&si_other, 0, sizeof(si_other));
			si_other.sin_family = AF_INET;
			si_other.sin_port = htons(intPort);
			si_other.sin_addr = address.address.sin_addr;
		}

	}
	//Destructor
	UDPClient::~UDPClient()
	{

		disconnect();
		if(active)
			close(s);

		safeDelete.acquire();
		popLock.acquire();

		//Clear the incoming packet queue
		UDPPacket* r;
		while(!incomingPackets.empty())
		{
			r = incomingPackets.pop();
			delete[] r->getData();
			delete r;
		}
		popLock.release();
		safeDelete.release();
	}
	//Begins the connection
	void UDPClient::connect()
	{
		if(!connected)
		{
			connected = true;
			spawnThread(&recieveThreadClient,this);
			spawnThread(&UDPTimer,this);
		}
	}
	//Forces the disconnection
	void UDPClient::disconnect()
	{
		connected = false;
	}
	//The received loop
	void UDPClient::recieveLoop()
	{
		char buf[BUFLEN];
		while(connected)
		{
			 memset(buf,'\0', BUFLEN);
			//try to receive some data, this is a blocking call
			if (connected && recvfrom(s, buf, BUFLEN, 0, (sockaddr *) &si_other, (socklen_t*) &slen) > 0)
			{
				safeDelete.acquire();
				if(connected)
				{
					conTrack = resetVal;
					IPAddress temp;
					temp.address = si_other;
					UDPPacket* pck = new UDPPacket((uint8_t*) buf, &temp,si_other.sin_port);
					incomingPackets.push(pck);

					//Raise the recieved event
					if(recieveFunction!=NULL)
						recieveFunction(recievePointer);
				}
				safeDelete.release();
			}
		}
	}
	//The timer tick
	void UDPClient::tick()
	{
		while(connected)
		{
			if(conTrack>0)
				conTrack-=10;
			sleep(10);
		}
	}

//Get Functions--------------------------------------------------------------------------------------------------------------------------
	
	//Returns if the client is active
	bool UDPClient::getActive()
	{
		return active;
	}
	//Returns if the client is currently connected
	bool UDPClient::getConnected()
	{
		return conTrack>0;
	}

//Set Functions--------------------------------------------------------------------------------------------------------------------------

	//Sets the value of the reset timer
	void UDPClient::setReset(int x)
	{
		if(x>=20)
			resetVal = x;
	}
	//Sets the recieved event functions
	void UDPClient::setRecieveEvent(void (*func)(void *),void *ptr)
	{
		recieveFunction = func;
		recievePointer = ptr;
	}

//Send/Recieve---------------------------------------------------------------------------------------------------------------------------

	//Checks for available data
	bool UDPClient::available()
	{
		return !incomingPackets.empty();
	}
	//Send data
	bool UDPClient::send(UDPPacket* pck)
	{
		char* sent = (char*) pck->sendData();
		
		if (sendto(s, sent, pck->getLength()+2, 0 , (sockaddr *) &si_other, slen) == pck->getLength()+2)
			return true;
		return false;
	}
	//Returns the top of the recieved stack
	UDPPacket* UDPClient::recieve()
	{
		popLock.acquire();
		if(!connected||incomingPackets.empty())
		{
			popLock.release();
			return NULL;
		}

		UDPPacket* temp =  incomingPackets.pop();
		popLock.release();

		return temp;
	}

//UDP Server-----------------------------------------------------------------------------------------------------------------------------
	
	//The static function which begins the recieve loop
	static void recieveThreadServer(void* ptr)
	{
		UDPServer * pthX = (UDPServer*)ptr;
		pthX->UDPServer::recieveLoop();
	}

//Constructor/Destructor-----------------------------------------------------------------------------------------------------------------

	//Constructor
	UDPServer::UDPServer(int port)
	{
		active = true;
		connected = false;
		intPort = port;

		recieveFunction = NULL;
		recievePointer = NULL;

		slen = sizeof(si_other);
	}
	//Destructor
	UDPServer::~UDPServer()
	{
		end();
		if(active)
			close(s);

		safeDelete.acquire();
		popLock.acquire();

		//Clear the incoming packet queue
		UDPPacket* r;
		while(!incomingPackets.empty())
		{
			r = incomingPackets.pop();
			delete [] r->getData();
			delete r;
		}
		popLock.release();
		safeDelete.release();
	}
	//Begins the server
	void UDPServer::start()
	{
		if(!connected)
		{
			//Create a socket
			if((s = socket(AF_INET , SOCK_DGRAM , 0 )) <0)
			{
				cerr<<"Could not create server socket"<<endl;;
				active = false;
			}
	    
			//Prepare the sockaddr_in structure
			if(active)
			{
				server.sin_family = AF_INET;
				server.sin_addr.s_addr = INADDR_ANY;
				server.sin_port = htons( intPort );
			}
			
			int temp = false;
			setsockopt(s, SOL_SOCKET,
			      SO_REUSEADDR,
			      (const char *) &temp, sizeof(temp));
			
			 //Bind
			if(bind(s ,(struct sockaddr *)&server , sizeof(server)) < 0)
				return;
			connected = true;
			spawnThread(&recieveThreadServer,this);
		}
	}
	//Shuts down the server
	void UDPServer::end()
	{
		if(connected)
		{
			connected = true;
		
			int temp = false;
			setsockopt(s, SOL_SOCKET,
			    SO_REUSEADDR,
			    (const char *) &temp, sizeof(temp));
		
			//Close the socket
			if(close(s)<0)
			{
				cerr<<"Unable to close socket!"<<endl;
				exit(EXIT_FAILURE);
			}
		}
	}
	//Recieve loop
	void UDPServer::recieveLoop()
	{
		char buf[BUFLEN];

		while(connected)
		{
			//clear the buffer by filling null, it might have previously received data
			memset(buf,'\0', BUFLEN);
         
			//try to receive some data, this is a blocking call
			if ((recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other,  (socklen_t*)&slen)) < 0)
			{
			    safeDelete.acquire();
				if(connected)
				{
					IPAddress temp;
					temp.address = si_other;
					UDPPacket* pck = new UDPPacket((uint8_t*) buf, &temp,si_other.sin_port);
					incomingPackets.push(pck);

					//Raise the recieved event
					if(recieveFunction!=NULL)
						recieveFunction(recievePointer);
				}
				safeDelete.release();
			}
		}
	}

//Get/Set Functions----------------------------------------------------------------------------------------------------------------------

	//Returns if the server is active
	bool UDPServer::getActive()
	{
		return active;
	}
	//Returns if the server is connected
	bool UDPServer::getConnected()
	{
		return connected;
	}
	//Sents the function to be triggered on reception
	void UDPServer::setRecieveEvent(void (*func)(void *),void *ptr)
	{
		recieveFunction = func;
		recievePointer = ptr;
	}

//Send/Recieve---------------------------------------------------------------------------------------------------------------------------

	//Sends a packet
	bool UDPServer::send(UDPPacket* pck)
	{
		char* sent = (char*) pck->sendData();
		si_other.sin_addr = pck->getAddress()->address.sin_addr;

		if (sendto(s, sent, pck->getLength()+2, 0, (struct sockaddr*) &si_other,  (socklen_t) slen) < 0)
			return true;
		return false;
	}
	//Checks if there are available packets
	bool UDPServer::available()
	{
		return !incomingPackets.empty();
	}
	//Recieves a packet
	UDPPacket* UDPServer::recieve()
	{
		popLock.acquire();
		if(!connected||incomingPackets.empty())
		{
			popLock.release();
			return NULL;
		}

		UDPPacket* temp =  incomingPackets.pop();
		popLock.release();

		return temp;
	}
#endif
