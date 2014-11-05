#!/bin/bash

g++ -o WifiRC -I ../CryptoGateway -pthread WifiRC.cpp AVL.cpp BaseController.cpp ConnectionManager.cpp Controller.cpp GenControls.cpp osFunctions.cpp RCControls.cpp RemoteController.cpp serialThread.cpp spinLock.cpp