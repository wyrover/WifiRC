//Primary author: Jonathan Bedard
//Confirmed working: 10/11/2014

#ifndef WIFIRC_CPP
#define WIFIRC_CPP

#include "osFunctions.h"
#include <stdio.h>
#include <iostream>

#include "CryptoGateway.h"
#include "CryptoGatewayComplete.h"
 
#define BUFLEN 512  //Max length of buffer
#define PORT 1024   //The port on which to listen for incoming data

using namespace std;

//The received event
static void wifirc_recieved(void * ptr)
{
	sleep(1000);
	cout<<"\tNew thread"<<endl;
}

#ifndef MAIN
#define MAIN

int main()
{
	//Testing thread memory
	cout<<"Begin test"<<endl;
	int x = 0;

	while(x<10000)
	{
		spawnThread(&wifirc_recieved, NULL);
		x++;
		sleep(100);
	}
}

#endif

#endif