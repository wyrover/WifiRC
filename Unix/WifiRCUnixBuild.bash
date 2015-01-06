#!/bin/bash

/bin/sh WifiRCUnixDeconstruct.bash
cd ..
cp -r Unix/osFunctions.cpp osFunctions.cpp
cp -r Unix/osFunctions.h osFunctions.h
cp -r Unix/Serial.h Serial.h
cp -r Unix/spinLock.cpp spinLock.cpp
cp -r Unix/spinLock.h spinLock.h
cp -r Unix/WifiRCCompile.bash WifiRCCompile.bash
cd Unix
