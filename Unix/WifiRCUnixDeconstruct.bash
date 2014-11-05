#!/bin/bash

cd ..

rm -f -r "WifiRC.vcxproj"
rm -f -r "WifiRC.vcxproj.filters"
rm -f -r "WifiRC.vcxproj.user"

rm -f -r "osFunctions.cpp"
rm -f -r "osFunctions.h"
rm -f -r "Serial.h"
rm -f -r "spinLock.cpp"
rm -f -r "spinLock.h"

rm -f -r "WifiRC"
rm -f -r "WifiRCCompile.bash"

cd Unix
