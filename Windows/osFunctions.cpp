//Primary author: Jonathan Bedard
//Confirmed working: 10/28/2014

/*
WINDOWS ONLY
*/

#ifndef OSFUNCTIONS_CPP
#define OSFUNCTIONS_CPP

#include "WifiRC.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "Serial.h"
#include "safeQueue.h"
#include "osFunctions.h"

//All other operating systems must have their own .cpp file created

//Serial Communication-------------------------------------------------------------------------------------------------------------------
	//Initializes the serial port
	Serial::Serial(char *portName, bool t)
	{
		track = t;
	    //We're not yet connected
	    connected = false;

	    //Try to connect to the given port throuh CreateFile
		hSerial = CreateFile(portName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
	    //Check if the connection was successfull
	    if(hSerial==INVALID_HANDLE_VALUE)
	    {
			if(track)
				cout<<"Connection to "<<portName<<" failed"<<endl;
			else
				sleep(10);
	    }
	    else
	    {
	        //If connected we try to set the comm parameters
	        DCB dcbSerialParams = {0};

	        //Try to get the current
		    if (!GetCommState(hSerial, &dcbSerialParams))
		    {
		        //If impossible, show an error
				if(track)
					cout<<"failed to get current serial parameters "<<" on "<<portName<<endl;
				else
					sleep(10);
			}
			else
			{
			    //Define serial connection parameters for the arduino board
			    dcbSerialParams.BaudRate=CBR_9600;
			    dcbSerialParams.ByteSize=8;
			    dcbSerialParams.StopBits=ONESTOPBIT;
				dcbSerialParams.Parity=NOPARITY;

				//Set the parameters and check for their proper application
				if(!SetCommState(hSerial, &dcbSerialParams))
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
	        CloseHandle(hSerial);
	    }
	}
	//Reads data from the serial port
	int Serial::ReadData(uint8_t *buffer, unsigned int nbChar)
	{
	    //Number of bytes we'll have read
	    DWORD bytesRead;
	    //Number of bytes we'll really ask to read
	    unsigned int toRead;
	
	    //Use the ClearCommError function to get status info on the Serial port
	    ClearCommError(this->hSerial, &this->errors, &this->status);

	    //Check if there is something to read
	    if(this->status.cbInQue>0)
	    {
	        //If there is we check if there is enough data to read the required number
	        //of characters, if not we'll read only the available characters to prevent
	        //locking of the application.
	        if(this->status.cbInQue>nbChar)
	        {
	            toRead = nbChar;
	        }
	        else
	        {
	            toRead = this->status.cbInQue;
	        }

	        //Try to read the require number of chars, and return the number of read bytes on success
	        if(ReadFile(this->hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0)
	        {
		        return bytesRead;
			}
		}

		//If nothing has been read, or that an error was detected return -1
		return -1;
	}
	//Writes data to the serial port
	bool Serial::WriteData(uint8_t *buffer, unsigned int nbChar)
	{
		DWORD bytesSend;

	    //Try to write the buffer on the Serial port
	    if(!WriteFile(this->hSerial, (void *)buffer, nbChar, &bytesSend, 0))
	    {
	        //In case it don't work get comm error and return false
	        ClearCommError(this->hSerial, &this->errors, &this->status);

	        return false;
	    }
		else
		    return true; 
	}
	//Tests if the serial port is connected
	bool Serial::IsConnected()
	{
		//Simply return the connection status
		if(this == NULL)
			return false;
		return this->connected;
	}


#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

//IPAddress Class------------------------------------------------------------------------------------------------------------------------

	//Constructor
	IPAddress::IPAddress()
	{
		inet_pton(AF_INET, "127.0.0.1", &(address));
	}
	IPAddress::IPAddress(string x)
	{
		inet_pton(AF_INET, x.c_str(), &(address));
	}
	IPAddress::IPAddress(IN_ADDR x)
	{
		address = x;
	}
	IPAddress::IPAddress(IPAddress* x)
	{
		address = x->address;
	}
	//Destructor
	IPAddress::~IPAddress()
	{
		//No clean up needed
	}
	//Returns the string of an IP address
	char* IPAddress::printAddress()
	{
		char* temp = inet_ntoa(address);
		int cnt = 0;

		while(cnt<79&&temp[cnt]!='/0')
		{
			name[cnt] = temp[cnt];
			cnt++;
		}
		name[cnt] = '/0';
		return name;
	}
	//Compares to IP addresses for equality
	int IPAddress::compare(const IPAddress* comp) const
	{
		uint32_t* me = (uint32_t*) &address;
		uint32_t* them = (uint32_t*) &(comp->address);
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
	}
	//Destructor
	myIPAddress::~myIPAddress()
	{
	}
	//Resets the address
	IN_ADDR myIPAddress::resetAddress()
	{
		IN_ADDR ret;
		inet_pton(AF_INET, "127.0.0.1", &(ret));
		if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR)
		{
			cerr << "Error " << WSAGetLastError() <<" when getting local host name." << endl;
			return ret;
		}

		struct hostent *phe = gethostbyname(hostName);
		if (phe == 0)
		{
			cerr << "Bad host lookup." << endl;
			return ret;
		}

		for (int i = 0; phe->h_addr_list[i] != 0; ++i)
		{
			memcpy(&ret, phe->h_addr_list[i], sizeof(struct in_addr));
		}

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
		char* temp = inet_ntoa(address);
		int cnt = 0;

		while(cnt<79&&temp[cnt]!='/0')
		{
			name[cnt] = temp[cnt];
			cnt++;
		}
		temp[cnt] = '/0';
		return name;
	}
	//Returns the host name of this computer
	char* myIPAddress::getHostName()
	{
		return hostName;
	}

//Data Packet Class----------------------------------------------------------------------------------------------------------------------

	//Receiving initializer
	UDPPacket::UDPPacket(byte* input, IPAddress* i, int p)
	{
		ip.address = i->address;
		//Unpack the type
		type = (uint8_t) input[0]>>1;
		type = type & 127;

		//Unpack the length
		length = (uint16_t) input[1];
		length = (uint16_t) length | ((input[0]&1)<<8);

		//Unpack the data
		data = new byte[length];
		in_or_out = true;
		int cnt = 0;

		while(cnt<length)
		{
			data[cnt] = input[cnt+2];
			cnt++;
		}
		port = p;
	}
	//Sending initializer
	UDPPacket::UDPPacket(byte* output, int l, int t, IPAddress* i,int p)
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
	byte* UDPPacket::getData()
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
	byte* UDPPacket::sendData()
	{
		byte* ret = new byte[length+2];
		int cnt = 0;

		//Transfer the data
		while(cnt<length)
		{
			ret[cnt+2] = data[cnt];
			cnt++;
		}

		//Pack the type and length
		ret[1] = (byte) length;
		ret[0] = (byte) (length>>8);
		ret[0] = (byte) ret[0] | (type<<1);

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
		if ( active&&(s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		{
			cerr<<"socket() failed with error code : "<<WSAGetLastError()<<endl;
			active = false;
		}
 
		if(active)
		{
			//setup address structure
			memset((char *) &si_other, 0, sizeof(si_other));
			si_other.sin_family = AF_INET;
			si_other.sin_port = htons(intPort);
			si_other.sin_addr = address.address;
		}

	}
	//Destructor
	UDPClient::~UDPClient()
	{

		disconnect();
		if(active)
			closesocket(s);

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
			if (connected && recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) != SOCKET_ERROR)
			{
				safeDelete.acquire();
				if(connected)
				{
					conTrack = resetVal;
					IPAddress temp;
					temp.address = si_other.sin_addr;
					UDPPacket* pck = new UDPPacket((byte*) buf, &temp,si_other.sin_port);
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
			Sleep(10);
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
		
		if (sendto(s, sent, pck->getLength()+2, 0 , (struct sockaddr *) &si_other, slen) != SOCKET_ERROR)
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
			closesocket(s);

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
			if((s = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
			{
				cerr<<"Could not create socket : "<< WSAGetLastError()<<endl;;
				active = false;
			}
     
			//Prepare the sockaddr_in structure
			if(active)
			{
				server.sin_family = AF_INET;
				server.sin_addr.s_addr = INADDR_ANY;
				server.sin_port = htons( intPort );
			}

			 //Bind
			if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
				return;

			connected = true;
			spawnThread(&recieveThreadServer,this);
		}
	}
	//Shuts down the server
	void UDPServer::end()
	{
		connected = false;

		if(closesocket(s) == SOCKET_ERROR)
		{
			cerr<<"Unable to close socket.  Error code : "<<WSAGetLastError()<<endl;
			exit(EXIT_FAILURE);
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
			if ((recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) != SOCKET_ERROR)
			{
			    safeDelete.acquire();
				if(connected)
				{
					IPAddress temp;
					temp.address = si_other.sin_addr;
					UDPPacket* pck = new UDPPacket((byte*) buf, &temp,si_other.sin_port);
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
		si_other.sin_addr = pck->getAddress()->address;

		if (sendto(s, sent, pck->getLength()+2, 0, (struct sockaddr*) &si_other, slen) != SOCKET_ERROR)
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